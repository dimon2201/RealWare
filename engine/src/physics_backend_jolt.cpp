// physics_backend_jolt.cpp

#include <cstdarg>
#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include "physics_backend_jolt.hpp"
#include "log.hpp"
#include "context.hpp"
#include "engine.hpp"
#include "capabilities.hpp"
#include "application.hpp"

using namespace JPH;
using namespace types;

static void TraceImpl(const char* inFMT, ...)
{
	va_list list;
	va_start(list, inFMT);
	char buffer[1024];
	vsnprintf(buffer, sizeof(buffer), inFMT, list);
	va_end(list);
	triton::Print(buffer);
}

static bool AssertFailedImpl(const char* inExpression, const char* inMessage, const char* inFile, uint inLine)
{
	std::ostringstream ss;
	ss << inFile << ":" << inLine << ": (" << inExpression << ") " << (inMessage != nullptr ? inMessage : "");
	triton::Print(ss);

	return true;
};

namespace Layers
{
	static constexpr ObjectLayer NON_MOVING = 0;
	static constexpr ObjectLayer MOVING = 1;
	static constexpr ObjectLayer NUM_LAYERS = 2;
};

class ObjectLayerPairFilterImpl : public ObjectLayerPairFilter
{
public:
	virtual bool ShouldCollide(ObjectLayer inObject1, ObjectLayer inObject2) const override
	{
		switch (inObject1)
		{
		case Layers::NON_MOVING:
			return inObject2 == Layers::MOVING; // Non moving only collides with moving
		case Layers::MOVING:
			return true; // Moving collides with everything
		default:
			JPH_ASSERT(false);
			return false;
		}
	}
};

namespace BroadPhaseLayers
{
	static constexpr BroadPhaseLayer NON_MOVING(0);
	static constexpr BroadPhaseLayer MOVING(1);
	static constexpr uint NUM_LAYERS(2);
};

class BPLayerInterfaceImpl final : public BroadPhaseLayerInterface
{
public:
	BPLayerInterfaceImpl()
	{
		mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
		mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
	}

	virtual uint GetNumBroadPhaseLayers() const override
	{
		return BroadPhaseLayers::NUM_LAYERS;
	}

	virtual BroadPhaseLayer GetBroadPhaseLayer(ObjectLayer inLayer) const override
	{
		JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
		return mObjectToBroadPhase[inLayer];
	}

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
	virtual const char* GetBroadPhaseLayerName(BroadPhaseLayer inLayer) const override
	{
		switch ((BroadPhaseLayer::Type)inLayer)
		{
			case (BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:	return "NON_MOVING";
			case (BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:		return "MOVING";
			default:													JPH_ASSERT(false); return "INVALID";
		}
	}
#endif

private:
	BroadPhaseLayer mObjectToBroadPhase[Layers::NUM_LAYERS];
};

class ObjectVsBroadPhaseLayerFilterImpl : public ObjectVsBroadPhaseLayerFilter
{
public:
	virtual bool ShouldCollide(ObjectLayer inLayer1, BroadPhaseLayer inLayer2) const override
	{
		switch (inLayer1)
		{
			case Layers::NON_MOVING:
				return inLayer2 == BroadPhaseLayers::MOVING;
			case Layers::MOVING:
				return true;
			default:
				JPH_ASSERT(false);
				return false;
		}
	}
};

class MyContactListener : public ContactListener
{
public:
	// See: ContactListener
	virtual ValidateResult OnContactValidate(const Body& inBody1, const Body& inBody2, RVec3Arg inBaseOffset, const CollideShapeResult& inCollisionResult) override
	{
		std::cout << "Contact validate callback" << std::endl;

		// Allows you to ignore a contact before it is created (using layers to not make objects collide is cheaper!)
		return ValidateResult::AcceptAllContactsForThisBodyPair;
	}

	virtual void OnContactAdded(const Body& inBody1, const Body& inBody2, const ContactManifold& inManifold, ContactSettings& ioSettings) override
	{
		std::cout << "A contact was added" << std::endl;
	}

	virtual void OnContactPersisted(const Body& inBody1, const Body& inBody2, const ContactManifold& inManifold, ContactSettings& ioSettings) override
	{
		std::cout << "A contact was persisted" << std::endl;
	}

	virtual void OnContactRemoved(const SubShapeIDPair& inSubShapePair) override
	{
		std::cout << "A contact was removed" << std::endl;
	}
};

// An example activation listener
class MyBodyActivationListener : public BodyActivationListener
{
public:
	virtual void OnBodyActivated(const BodyID& inBodyID, uint64 inBodyUserData) override
	{
		std::cout << "A body got activated" << std::endl;
	}

	virtual void OnBodyDeactivated(const BodyID& inBodyID, uint64 inBodyUserData) override
	{
		std::cout << "A body went to sleep" << std::endl;
	}
};

triton::XPhysicsBackendJolt::XPhysicsBackendJolt(cContext* context) : IPhysicsBackend(context)
{
	RegisterDefaultAllocator();
	Trace = TraceImpl;
	JPH_IF_ENABLE_ASSERTS(AssertFailed = AssertFailedImpl;)
	Factory::sInstance = new Factory();
	RegisterTypes();
}

triton::XPhysicsBackendJolt::~XPhysicsBackendJolt()
{
	UnregisterTypes();

	delete Factory::sInstance;
	Factory::sInstance = nullptr;
}

triton::SPhysicsWorldBackendData triton::XPhysicsBackendJolt::CreateWorld()
{
	const sCapabilities& caps = _context->GetSubsystem<CEngine>()->GetApplication()->GetCapabilities();

	TempAllocatorImpl* tempAllocator = new TempAllocatorImpl(caps.maxPhysicsTempBufferByteSize);
	JobSystemThreadPool* jobSystem = new JobSystemThreadPool(
		cMaxPhysicsJobs,
		cMaxPhysicsBarriers,
		thread::hardware_concurrency() - 1
	);
	BPLayerInterfaceImpl* broadPhaseLayerInterface = new BPLayerInterfaceImpl();
	ObjectVsBroadPhaseLayerFilterImpl* objectVsBroadphaseLayerFilter = new ObjectVsBroadPhaseLayerFilterImpl();
	ObjectLayerPairFilterImpl* objectVsObjectLayerFilter = new ObjectLayerPairFilterImpl();
	PhysicsSystem* physicsSystem = new PhysicsSystem();
	physicsSystem->Init(
		caps.maxPhysicsSimulationBodyCount,
		caps.maxPhysicsSimulationBodyMutexCount,
		caps.maxPhysicsSimulationBodyPairCount,
		caps.maxPhysicsSimulationContactConstraintCount,
		*broadPhaseLayerInterface,
		*objectVsBroadphaseLayerFilter,
		*objectVsObjectLayerFilter
	);
	MyBodyActivationListener* bodyActivationListener = new MyBodyActivationListener();
	physicsSystem->SetBodyActivationListener(bodyActivationListener);
	MyContactListener* contactListener = new MyContactListener();;
	physicsSystem->SetContactListener(contactListener);

	SPhysicsWorldBackendData pwbd;
	pwbd.systemObjects[(int)ESystemObjectIndices::TempAllocator] = (SPair("tempAllocator", tempAllocator));
	pwbd.systemObjects[(int)ESystemObjectIndices::JobSystem] = (SPair("jobSystem", jobSystem));
	pwbd.systemObjects[(int)ESystemObjectIndices::BroadPhaseLayerInterface] = (SPair("broadPhaseLayerInterface", broadPhaseLayerInterface));
	pwbd.systemObjects[(int)ESystemObjectIndices::ObjectVsBroadphaseLayerFilter] = (SPair("objectVsBroadphaseLayerFilter", objectVsBroadphaseLayerFilter));
	pwbd.systemObjects[(int)ESystemObjectIndices::ObjectVsObjectLayerFilter] = (SPair("objectVsObjectLayerFilter", objectVsObjectLayerFilter));
	pwbd.systemObjects[(int)ESystemObjectIndices::PhysicsSystem] = (SPair("physicsSystem", physicsSystem));
	pwbd.systemObjects[(int)ESystemObjectIndices::BodyActivationListener] = (SPair("bodyActivationListener", bodyActivationListener));
	pwbd.systemObjects[(int)ESystemObjectIndices::ContactListener] = (SPair("contactListener", contactListener));
	pwbd.systemObjectCount = 8;
	pwbd.worldIndex = 5;

	return pwbd;
}

void triton::XPhysicsBackendJolt::DestroyWorld(const SPhysicsWorldBackendData& world)
{
	for (usize i = 0; i < world.systemObjectCount; i++)
	{
		if (world.systemObjects[i].ptr)
		{
			if (world.systemObjects[i].tag == "tempAllocator")
				delete (TempAllocatorImpl*)world.systemObjects[i].ptr;
			else if (world.systemObjects[i].tag == "jobSystem")
				delete (JobSystemThreadPool*)world.systemObjects[i].ptr;
			else if (world.systemObjects[i].tag == "broadPhaseLayerInterface")
				delete (BPLayerInterfaceImpl*)world.systemObjects[i].ptr;
			else if (world.systemObjects[i].tag == "objectVsBroadphaseLayerFilter")
				delete (ObjectVsBroadPhaseLayerFilterImpl*)world.systemObjects[i].ptr;
			else if (world.systemObjects[i].tag == "objectVsObjectLayerFilter")
				delete (ObjectLayerPairFilterImpl*)world.systemObjects[i].ptr;
			else if (world.systemObjects[i].tag == "physicsSystem")
				delete (PhysicsSystem*)world.systemObjects[i].ptr;
			else if (world.systemObjects[i].tag == "bodyActivationListener")
				delete (MyBodyActivationListener*)world.systemObjects[i].ptr;
			else if (world.systemObjects[i].tag == "contactListener")
				delete (MyContactListener*)world.systemObjects[i].ptr;
		}
	}
}

triton::SPhysicsShapeBackendData triton::XPhysicsBackendJolt::CreateBoxShape(
	const SPhysicsWorldBackendData& world,
	const SPhysicsShapeDesc& desc
)
{
	PhysicsSystem* ps = (PhysicsSystem*)world.systemObjects[world.worldIndex].ptr;
	BoxShapeSettings shapeSettings(Vec3(
		desc.halfExtent.GetX(),
		desc.halfExtent.GetY(),
		desc.halfExtent.GetZ()
	));
	shapeSettings.SetEmbedded();
	ShapeSettings::ShapeResult shapeResult = shapeSettings.Create();
	ShapeRefC* shape = _context->Create<ShapeRefC>(shapeResult.Get());

	SPhysicsShapeBackendData psbd;
	psbd.shape = (void*)shape;

	return psbd;
}

triton::SPhysicsShapeBackendData triton::XPhysicsBackendJolt::CreateSphereShape(const SPhysicsShapeDesc& desc)
{
	return {};
}

triton::SPhysicsShapeBackendData triton::XPhysicsBackendJolt::CreateCapsuleShape(const SPhysicsShapeDesc& desc)
{
	return {};
}

triton::SPhysicsShapeBackendData triton::XPhysicsBackendJolt::CreateCylinderShape(const SPhysicsShapeDesc& desc)
{
	return {};
}

triton::SPhysicsShapeBackendData triton::XPhysicsBackendJolt::CreateConvexHullShape(const SPhysicsShapeDesc& desc)
{
	return {};
}

triton::SPhysicsShapeBackendData triton::XPhysicsBackendJolt::CreateTriangleMeshShape(const SPhysicsShapeDesc& desc)
{
	return {};
}

triton::SPhysicsShapeBackendData triton::XPhysicsBackendJolt::CreateHeightFieldShape(const SPhysicsShapeDesc& desc)
{
	return {};
}

void triton::XPhysicsBackendJolt::DestroyShape(const SPhysicsShapeBackendData& shape)
{
	if (shape.shape)
		_context->Destroy<ShapeRefC>((ShapeRefC*)shape.shape);
}

triton::SPhysicsRigidBodyBackendData triton::XPhysicsBackendJolt::CreateRigidBody(
	const SPhysicsWorldBackendData& world,
	const SPhysicsRigidBodyDesc& desc,
	const SPhysicsShapeBackendData& shape
)
{
	PhysicsSystem* ps = (PhysicsSystem*)world.systemObjects[world.worldIndex].ptr;
	BodyInterface& bi = ps->GetBodyInterface();
	ShapeRefC* rbShape = (ShapeRefC*)shape.shape;

	JPH::EMotionType mt = JPH::EMotionType::Static;
	switch (desc.motionType)
	{
		case EMotionType::Static:    mt = JPH::EMotionType::Static; break;
		case EMotionType::Kinematic: mt = JPH::EMotionType::Kinematic; break;
		case EMotionType::Dynamic:   mt = JPH::EMotionType::Dynamic; break;
	}
	BodyCreationSettings rbSettings(
		*rbShape,
		RVec3(desc.worldPosition.GetX(), desc.worldPosition.GetY(), desc.worldPosition.GetZ()),
		Quat(desc.worldRotation.GetX(), desc.worldRotation.GetY(), desc.worldRotation.GetZ(), desc.worldRotation.GetW()),
		mt,
		desc.properties[(int)ERigidBodyPropertyIndices::ObjectLayer]
	);
	Body* rb = bi.CreateBody(rbSettings);
	bi.AddBody(rb->GetID(), (EActivation)desc.properties[(int)ERigidBodyPropertyIndices::ActivationMode]);
	BodyID* rbid = _context->Create<BodyID>(rb->GetID());

	SPhysicsRigidBodyBackendData prbbd;
	prbbd.body = (void*)rbid;

	return prbbd;
}

void triton::XPhysicsBackendJolt::DestroyRigidBody(
	const SPhysicsWorldBackendData& world, 
	const SPhysicsRigidBodyBackendData& body
)
{
	if (body.body)
	{
		PhysicsSystem* ps = (PhysicsSystem*)world.systemObjects[world.worldIndex].ptr;
		BodyInterface& bi = ps->GetBodyInterface();
		BodyID& bid = *(BodyID*)body.body;
		bi.RemoveBody(bid);
		bi.DestroyBody(bid);
		_context->Destroy<BodyID>((BodyID*)body.body);
	}
}

triton::SPhysicsCharacterControllerBackendData triton::XPhysicsBackendJolt::CreateCharacterController(const SPhysicsCharacterControllerDesc& desc)
{
	return {};
}

void triton::XPhysicsBackendJolt::DestroyCharacterController(const SPhysicsCharacterControllerBackendData& controller)
{
}

triton::SPhysicsConstraintBackendData triton::XPhysicsBackendJolt::CreateFixedConstraint(const SPhysicsConstraintDesc& desc)
{
	return {};
}

triton::SPhysicsConstraintBackendData triton::XPhysicsBackendJolt::CreateHingeConstraint(const SPhysicsConstraintDesc& desc)
{
	return {};
}

triton::SPhysicsConstraintBackendData triton::XPhysicsBackendJolt::CreateSliderConstraint(const SPhysicsConstraintDesc& desc)
{
	return {};
}

triton::SPhysicsConstraintBackendData triton::XPhysicsBackendJolt::CreateDistanceConstraint(const SPhysicsConstraintDesc& desc)
{
	return {};
}

triton::SPhysicsConstraintBackendData triton::XPhysicsBackendJolt::CreateConeConstraint(const SPhysicsConstraintDesc& desc)
{
	return {};
}

triton::SPhysicsConstraintBackendData triton::XPhysicsBackendJolt::CreateSwingTwistConstraint(const SPhysicsConstraintDesc& desc)
{
	return {};
}

void triton::XPhysicsBackendJolt::DestroyConstraint(const SPhysicsConstraintBackendData& constraint)
{
}

triton::SPhysicsMaterialBackendData triton::XPhysicsBackendJolt::CreateMaterial(const SPhysicsMaterialDesc& desc)
{
	return {};
}

void triton::XPhysicsBackendJolt::DestroyMaterial(const SPhysicsMaterialBackendData& material)
{
}

void triton::XPhysicsBackendJolt::OptimizeAccelerationStructures(const SPhysicsWorldBackendData& world)
{
	PhysicsSystem* ps = (PhysicsSystem*)world.systemObjects[world.worldIndex].ptr;
	ps->OptimizeBroadPhase();
}

void triton::XPhysicsBackendJolt::Update(
	const SPhysicsWorldBackendData& world,
	f32 deltaTime,
	s32 collisionStepCount
)
{
	PhysicsSystem* ps = (PhysicsSystem*)world.systemObjects[world.worldIndex].ptr;
	TempAllocatorImpl* ta = (TempAllocatorImpl*)world.systemObjects[(int)ESystemObjectIndices::TempAllocator].ptr;
	JobSystemThreadPool* js = (JobSystemThreadPool*)world.systemObjects[(int)ESystemObjectIndices::JobSystem].ptr;
	ps->Update(deltaTime, collisionStepCount, ta, js);
}