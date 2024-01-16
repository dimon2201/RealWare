#include <windows.h>
#include <cooking/PxCooking.h>
#include "render_manager.hpp"
#include "physics_manager.hpp"

using namespace physx;

namespace realware
{
    namespace physics
    {
        PxFilterFlags FilterShader(
            PxFilterObjectAttributes attributes0, PxFilterData filterData0,
            PxFilterObjectAttributes attributes1, PxFilterData filterData1,
            PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
        {
            if (PxFilterObjectIsTrigger(attributes0) || PxFilterObjectIsTrigger(attributes1))
            {
                pairFlags = PxPairFlag::eTRIGGER_DEFAULT;
                return PxFilterFlag::eDEFAULT;
            }
            pairFlags = PxPairFlag::eCONTACT_DEFAULT;

            if ((filterData0.word0 & filterData1.word0) && (filterData1.word1 & filterData0.word1))
                pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND;

            return PxFilterFlag::eDEFAULT;

        }

        void mPhysics::Init()
        {
            m_allocator = new cAllocator();
            m_error = new cError();
            m_cpuDispatcher = new cCPUDispatcher();
            m_simulationEvent = new cSimulationEvent();

            m_foundation = PxCreateFoundation(
                PX_PHYSICS_VERSION,
                *m_allocator,
                *m_error
            );

            if (m_foundation == nullptr)
            {
                MessageBox(0, "Failed to initialize PhysXFoundation!", "Error", MB_ICONERROR);
                return;
            }

            m_physics = PxCreatePhysics(
                PX_PHYSICS_VERSION,
                *m_foundation,
                PxTolerancesScale(),
                false,
                nullptr
            );

            if (m_physics == nullptr)
            {
                MessageBox(0, "Failed to initialize PhysXPhysics!", "Error", MB_ICONERROR);
                return;
            }
        }

        void mPhysics::Free()
        {
            m_physics->release();
            m_foundation->release();
        }

        void mPhysics::Update()
        {
            // Actors
            for (auto& pair : m_actors)
            {
                core::sCTransform* transformComponent = pair.Scene->Get<core::sCTransform>(pair.Entity);
                core::sCPhysicsActor* actorComponent = pair.Scene->Get<core::sCPhysicsActor>(pair.Entity);

                PxTransform actorTransform = ((PxRigidDynamic*)actorComponent->Actor)->getGlobalPose();
                glm::vec3 actorEuler = glm::eulerAngles(
                    glm::quat(
                        actorTransform.q.w,
                        actorTransform.q.x,
                        actorTransform.q.y,
                        actorTransform.q.z
                    )
                );

                {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    transformComponent->Position = glm::vec3(actorTransform.p.y, actorTransform.p.x, actorTransform.p.z);
                    transformComponent->Rotation = glm::vec3(actorEuler.y, actorEuler.x, actorEuler.z);
                }
            }

            // Controllers
            for (auto& pair : m_controllers)
            {
                core::sCTransform* transformComponent = pair.Scene->Get<core::sCTransform>(pair.Entity);
                core::sCPhysicsCharacterController* controllerComponent =
                    pair.Scene->Get<core::sCPhysicsCharacterController>(pair.Entity);

                if (controllerComponent->IsGravityEnabled == core::K_TRUE) {
                    SetCharacterControllerMovement(
                        pair, glm::vec3(0.0f, -controllerComponent->GravitySpeed, 0.0f)
                    );
                }

                PxExtendedVec3 position = controllerComponent->Controller->getPosition();
                
                {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    transformComponent->Position = glm::vec3(
                        position.y, position.x, position.z
                    );
                }
            }

            // Scenes
            for (auto& pair : m_scenes)
            {
                core::sCPhysicsScene* component = pair.Scene->Get<core::sCPhysicsScene>(pair.Entity);
                component->Scene->simulate(1.0f / 60.0f);
                component->Scene->fetchResults(true);
            }
        }

        core::sCPhysicsScene* mPhysics::AddScene(const core::sEntityScenePair& scene)
        {
            PxSceneDesc sceneDesc(m_physics->getTolerancesScale());
            sceneDesc.gravity = PxVec3(-9.81f, 0.0f, 0.0f);
            sceneDesc.cpuDispatcher = m_cpuDispatcher;
            sceneDesc.simulationEventCallback = m_simulationEvent;
            sceneDesc.filterShader = FilterShader;

            core::sCPhysicsScene* component = scene.Scene->Add<core::sCPhysicsScene>(scene.Entity);
            component->Scene = m_physics->createScene(sceneDesc);
            component->ControllerManager = PxCreateControllerManager(*component->Scene);

            m_scenes.push_back(scene);

            return component;
        }

        core::sCPhysicsActor* mPhysics::AddActor(
            const core::sEntityScenePair& scene,
            const core::sEntityScenePair& actor,
            mPhysics::eActorDescriptor actorDesc,
            mPhysics::eShapeDescriptor shapeDesc,
            const glm::vec4& extents,
            render::sVertexBufferGeometry* geometry
        )
        {
            core::sCTransform* transformComponent = actor.Scene->Get<core::sCTransform>(actor.Entity);

            glm::vec3& entityPosition = transformComponent->Position;
            glm::vec3 position = glm::vec3(entityPosition.y, entityPosition.x, entityPosition.z);

            // Creating material
            PxMaterial* physicsMaterial = m_physics->createMaterial(0.05f, 0.05f, 0.05f);

            // Creation of actor
            PxActor* physicsActor = nullptr;
            
            switch (actorDesc)
            {

            case mPhysics::eActorDescriptor::STATIC:
                physicsActor = m_physics->createRigidStatic(PxTransform(position.x, position.y, position.z));
                break;

            case mPhysics::eActorDescriptor::DYNAMIC:
                physicsActor = m_physics->createRigidDynamic(PxTransform(position.x, position.y, position.z));
                ((PxRigidDynamic*)physicsActor)->setAngularDamping(0.75f);
                ((PxRigidDynamic*)physicsActor)->setLinearVelocity(PxVec3(0.0f, 0.0f, 0.0f));
                break;

            }

            if (physicsActor == nullptr) { return nullptr; }

            core::sCPhysicsScene* sceneComponent = scene.Scene->Get<core::sCPhysicsScene>(scene.Entity);
            sceneComponent->Scene->addActor(*physicsActor);

            // Shape creation
            PxShape* physicsShape = nullptr;

            switch (shapeDesc)
            {
                case mPhysics::eShapeDescriptor::SPHERE:
                {
                    physicsShape = m_physics->createShape(
                        PxSphereGeometry(extents.x),
                        *physicsMaterial
                    );
                    break;
                }

                case mPhysics::eShapeDescriptor::CAPSULE:
                {
                    physicsShape = m_physics->createShape(
                        PxCapsuleGeometry(extents.x, extents.y),
                        *physicsMaterial
                    );
                    break;
                }

                case mPhysics::eShapeDescriptor::BOX:
                {
                    physicsShape = m_physics->createShape(
                        PxBoxGeometry(extents.x, extents.y, extents.z),
                        *physicsMaterial
                    );
                    break;
                }

                case mPhysics::eShapeDescriptor::PLANE:
                {
                    physicsShape = m_physics->createShape(
                        PxPlaneGeometry(),
                        *physicsMaterial
                    );
                    break;
                }

                case mPhysics::eShapeDescriptor::TRIANGLE_MESH:
                {
                    if (geometry->Format !=
                        render::sVertexBufferGeometry::eFormat::POSITION_TEXCOORD_NORMAL_VEC3_VEC2_VEC3)
                    {
                        MessageBox(0, "PhysX shape 'TRIANGLE_MESH' can only be created with geometry of 'POSITION_TEXCOORD_NORMAL_VEC3_VEC2_VEC3' format!", "Error", MB_ICONERROR);
                        return nullptr;
                    }

                    PxVec3* verticesPX = new PxVec3[geometry->VertexCount];
                    for (core::s32 i = 0; i < geometry->VertexCount; i++) {
                        verticesPX[i] = PxVec3(
                            ((render::sVertex*)geometry->VertexPtr)[i].Position.y,
                            ((render::sVertex*)geometry->VertexPtr)[i].Position.x,
                            ((render::sVertex*)geometry->VertexPtr)[i].Position.z
                        );
                    }

                    PxTriangleMeshDesc meshDesc;
                    meshDesc.points.count = geometry->VertexCount;
                    meshDesc.points.stride = sizeof(PxVec3);
                    meshDesc.points.data = verticesPX;

                    meshDesc.triangles.count = geometry->IndexCount / 3;
                    meshDesc.triangles.stride = 3 * sizeof(PxU32);
                    meshDesc.triangles.data = geometry->IndexPtr;

                    PxTolerancesScale scale(1.0f, 10.0f);
                    PxCookingParams params(scale);
                    PxDefaultMemoryOutputStream writeBuffer;
                    PxTriangleMeshCookingResult::Enum result;
                    bool status = PxCookTriangleMesh(params, meshDesc, writeBuffer, &result);
                    if (!status || result == PxTriangleMeshCookingResult::eFAILURE)
                    {
                        MessageBox(0, "PhysX can't cook triangle mesh!", "Error", MB_ICONERROR);
                        return nullptr;
                    }

                    PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
                    PxTriangleMesh* triangleMesh = m_physics->createTriangleMesh(readBuffer);
                    
                    ((PxRigidDynamic*)physicsActor)->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
                    
                    physicsShape = m_physics->createShape(
                        PxTriangleMeshGeometry(
                            triangleMesh,
                            PxMeshScale(PxVec3(1.0f, 1.0f, 1.0f))
                        ),
                        *physicsMaterial,
                        false,
                        PxShapeFlag::eSIMULATION_SHAPE
                    );

                    delete[] verticesPX;

                    break;
                }
            }

            if (physicsShape == nullptr) { return nullptr; }

            physicsShape->setContactOffset(0.1f);
            physicsShape->setRestOffset(0.05f);

            PxFilterData FilterData;
            FilterData.word0 = 0; FilterData.word1 = 0; FilterData.word2 = 0; FilterData.word3 = 0;
            physicsShape->setSimulationFilterData(FilterData);

            switch (actorDesc)
            {

            case mPhysics::eActorDescriptor::STATIC:
                ((PxRigidStatic*)physicsActor)->attachShape(*physicsShape);
                break;

            case mPhysics::eActorDescriptor::DYNAMIC:
                ((PxRigidDynamic*)physicsActor)->attachShape(*physicsShape);
                break;

            }
            
            core::sCPhysicsActor* component = actor.Scene->Add<core::sCPhysicsActor>(actor.Entity);
            component->Actor = physicsActor;
            component->Material = physicsMaterial;

            m_actors.push_back(actor);

            return component;
        }

        core::sCPhysicsCharacterController* mPhysics::AddCharacterController(
            const core::sEntityScenePair& scene,
            const core::sEntityScenePair& controller,
            mPhysics::eShapeDescriptor shapeDesc,
            const glm::vec4& extents
        )
        {
            core::sCPhysicsScene* sceneComponent = scene.Scene->Get<core::sCPhysicsScene>(scene.Entity);
            core::sCTransform* transformComponent = controller.Scene->Get<core::sCTransform>(controller.Entity);
            PxControllerManager* controllerManager = sceneComponent->ControllerManager;
            
            PxMaterial* physicsMaterial = m_physics->createMaterial(0.05f, 0.05f, 0.05f);

            PxController* physicsController = nullptr;
            switch (shapeDesc)
            {
                case mPhysics::eShapeDescriptor::CAPSULE:
                {
                    PxCapsuleControllerDesc desc = PxCapsuleControllerDesc();
                    desc.setToDefault();
                    desc.radius = extents.x;
                    desc.height = extents.y;
                    desc.position = PxExtendedVec3(
                        transformComponent->Position.y,
                        transformComponent->Position.x,
                        transformComponent->Position.z
                    );
                    desc.upDirection = PxVec3(1.0f, 0.0f, 0.0f);
                    desc.contactOffset = 0.01;
                    desc.stepOffset = desc.height * 0.25f;
                    desc.slopeLimit = glm::cos(glm::radians(45.0f));
                    desc.material = physicsMaterial;

                    physicsController = controllerManager->createController(desc);
                    break;
                }

                case mPhysics::eShapeDescriptor::BOX:
                {
                    PxBoxControllerDesc desc = PxBoxControllerDesc();
                    desc.halfForwardExtent = extents.x;
                    desc.halfSideExtent = extents.y;
                    desc.halfHeight = extents.z;
                    desc.stepOffset = desc.halfHeight * 0.25f;
                    desc.slopeLimit = glm::cos(glm::radians(45.0f));
                    desc.material = physicsMaterial;

                    physicsController = controllerManager->createController(desc);
                    break;
                }
            }

            core::sCPhysicsCharacterController* controllerComponent =
                controller.Scene->Add<core::sCPhysicsCharacterController>(controller.Entity);
            controllerComponent->Controller = physicsController;
            controllerComponent->Material = physicsMaterial;

            m_controllers.push_back(controller);
            
            return controllerComponent;
        }

        void mPhysics::SetForce(const core::sEntityScenePair& actor, const glm::vec3& force)
        {
            core::sCPhysicsActor* component = actor.Scene->Get<core::sCPhysicsActor>(actor.Entity);
            ((PxRigidDynamic*)component->Actor)->setForceAndTorque(
                PxVec3(force.y, force.x, force.z),
                PxVec3(0.0f, 0.0f, 0.0f)
            );
        }

        void mPhysics::SetCharacterControllerMovement(
            const core::sEntityScenePair& controller,
            const glm::vec3& direction
        )
        {
            core::sCPhysicsCharacterController* controllerComponent =
                controller.Scene->Get<core::sCPhysicsCharacterController>(controller.Entity);

            PxControllerFilters filters = PxControllerFilters();
            controllerComponent->Controller->move(
                PxVec3(direction.y, direction.x, direction.z),
                0.01f,
                1.0f,
                filters
            );
        }
    }
}