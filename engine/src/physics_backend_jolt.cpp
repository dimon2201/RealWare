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

	BodyInterface& body_interface = physics_system.GetBodyInterface();

	BoxShapeSettings floor_shape_settings(Vec3(100.0f, 1.0f, 100.0f));
	floor_shape_settings.SetEmbedded();

	ShapeSettings::ShapeResult floor_shape_result = floor_shape_settings.Create();
	ShapeRefC floor_shape = floor_shape_result.Get();

	BodyCreationSettings floor_settings(floor_shape, RVec3(0.0f, -1.0f, 0.0f), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);

	Body* floor = body_interface.CreateBody(floor_settings);

	body_interface.AddBody(floor->GetID(), EActivation::DontActivate);

	BodyCreationSettings sphere_settings(new SphereShape(0.5f), RVec3(0.0f, 2.0f, 0.0f), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
	BodyID sphere_id = body_interface.CreateAndAddBody(sphere_settings, EActivation::Activate);

	body_interface.SetLinearVelocity(sphere_id, Vec3(0.0f, -5.0f, 0.0f));

	const float cDeltaTime = 1.0f / 60.0f;

	physics_system.OptimizeBroadPhase();

	uint step = 0;
	while (body_interface.IsActive(sphere_id))
	{
		++step;

		RVec3 position = body_interface.GetCenterOfMassPosition(sphere_id);
		Vec3 velocity = body_interface.GetLinearVelocity(sphere_id);

		const int cCollisionSteps = 1;

		physics_system.Update(cDeltaTime, cCollisionSteps, &temp_allocator, &job_system);
	}

	body_interface.RemoveBody(sphere_id);
	body_interface.DestroyBody(sphere_id);

	body_interface.RemoveBody(floor->GetID());
	body_interface.DestroyBody(floor->GetID());
}

triton::XPhysicsBackendJolt::~XPhysicsBackendJolt()
{
	UnregisterTypes();

	delete Factory::sInstance;
	Factory::sInstance = nullptr;
}

triton::SPhysicsWorldBackendData triton::XPhysicsBackendJolt::CreateWorld()
{
	const sCapabilities* caps = _context->GetSubsystem<cEngine>()->GetCapabilities();

	TempAllocatorImpl* tempAllocator = new TempAllocatorImpl(caps->maxPhysicsTempBufferByteSize);
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
		caps->maxPhysicsSimulationBodyCount,
		caps->maxPhysicsSimulationBodyMutexCount,
		caps->maxPhysicsSimulationBodyPairCount,
		caps->maxPhysicsSimulationContactConstraintCount,
		*broadPhaseLayerInterface,
		*objectVsBroadphaseLayerFilter,
		*objectVsObjectLayerFilter
	);
	MyBodyActivationListener* bodyActivationListener = new MyBodyActivationListener();
	physicsSystem->SetBodyActivationListener(bodyActivationListener);
	MyContactListener* contactListener = new MyContactListener();;
	physicsSystem->SetContactListener(contactListener);

	SPhysicsWorldBackendData pwbd;
	pwbd.systemObjects[0] = (SPair("tempAllocator", tempAllocator));
	pwbd.systemObjects[1] = (SPair("jobSystem", jobSystem));
	pwbd.systemObjects[2] = (SPair("broadPhaseLayerInterface", broadPhaseLayerInterface));
	pwbd.systemObjects[3] = (SPair("objectVsBroadphaseLayerFilter", objectVsBroadphaseLayerFilter));
	pwbd.systemObjects[4] = (SPair("objectVsObjectLayerFilter", objectVsObjectLayerFilter));
	pwbd.systemObjects[5] = (SPair("physicsSystem", physicsSystem));
	pwbd.systemObjects[6] = (SPair("bodyActivationListener", bodyActivationListener));
	pwbd.systemObjects[7] = (SPair("contactListener", contactListener));
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

triton::SPhysicsShapeBackendData triton::XPhysicsBackendJolt::CreateBoxShape(const SPhysicsShapeDesc& desc)
{
	return {};
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
}

triton::SPhysicsRigidBodyBackendData triton::XPhysicsBackendJolt::CreateRigidBody(
	const SPhysicsRigidBodyDesc& desc,
	const SPhysicsShapeBackendData& shape
)
{
	return {};
}

void triton::XPhysicsBackendJolt::DestroyRigidBody(const SPhysicsRigidBodyBackendData& body)
{
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