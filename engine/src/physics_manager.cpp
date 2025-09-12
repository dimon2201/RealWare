#include <windows.h>
#include <cooking/PxCooking.h>
#include "application.hpp"
#include "render_manager.hpp"
#include "physics_manager.hpp"

using namespace physx;

namespace realware
{
    using namespace app;
    using namespace game;
    using namespace types;

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

        mPhysics::mPhysics(const cApplication* const app) :
            _app((cApplication*)app),
            _allocator(new cAllocator()),
            _error(new cError()),
            _cpuDispatcher(new cCPUDispatcher()),
            _simulationEvent(new cSimulationEvent()),
            _sceneCount(0),
            _substanceCount(0)
        {
            _foundation = PxCreateFoundation(PX_PHYSICS_VERSION, *_allocator, *_error);
            if (_foundation == nullptr)
            {
                MessageBox(0, "Failed to initialize PhysXFoundation!", "Error", MB_ICONERROR);
                return;
            }

            _physics = PxCreatePhysics(PX_PHYSICS_VERSION, *_foundation, PxTolerancesScale(), false, nullptr);
            if (_physics == nullptr)
            {
                MessageBox(0, "Failed to initialize PhysXPhysics!", "Error", MB_ICONERROR);
                return;
            }

            auto desc = _app->GetDesc();
            _scenes.resize(desc->MaxPhysicsSceneCount);
            _substances.resize(desc->MaxPhysicsSubstanceCount);
            _actors.resize(desc->MaxPhysicsActorCount);
            _controllers.resize(desc->MaxPhysicsControllerCount);
        }

        mPhysics::~mPhysics()
        {
            _physics->release();
            _foundation->release();
            delete _simulationEvent;
            delete _cpuDispatcher;
            delete _error;
            delete _allocator;
        }

        cSimulationScene* mPhysics::AddScene(const std::string& id, const glm::vec3& gravity)
        {
            PxSceneDesc sceneDesc(_physics->getTolerancesScale());
            sceneDesc.gravity = PxVec3(gravity.x, gravity.y, gravity.z);
            sceneDesc.cpuDispatcher = _cpuDispatcher;
            sceneDesc.filterShader = PxDefaultSimulationFilterShader;

            PxScene* scene = _physics->createScene(sceneDesc);

            PxControllerManager* controllerManager = PxCreateControllerManager(*scene);

            _scenes[_sceneCount] = cSimulationScene(id, scene, controllerManager);
            _sceneCount += 1;

            return &_scenes[_sceneCount - 1];
        }

        cSubstance* mPhysics::AddSubstance(const std::string& id, const glm::vec3& params)
        {
            PxMaterial* material = _physics->createMaterial(params.x, params.y, params.z); // (staticFriction, dynamicFriction, restitution)

            _substances[_substanceCount] = cSubstance(id, material);
            _substanceCount += 1;

            return &_substances[_substanceCount - 1];
        }

        cController* mPhysics::AddController(const std::string& id, const f32 eyeHeight, const f32 height, const f32 radius, const render::sTransform* const transform, const glm::vec3& up, const cSimulationScene* const scene, const cSubstance* const substance)
        {
            glm::vec3 position = transform->Position;

            PxCapsuleControllerDesc desc;
            desc.height = height;
            desc.radius = radius;
            desc.position = PxExtendedVec3(position.y, position.x, position.z);
            desc.stepOffset = 0.5f;       // максимальный шаг при коллизии
            desc.slopeLimit = cosf(PxPi / 4.0f); // ограничение наклона поверхности
            desc.contactOffset = 0.01f;   // минимальное расстояние до коллизий
            desc.upDirection = PxVec3(up.y, up.x, up.z); // направление вверх
            desc.material = substance->GetSubstance();

            PxController* controller = scene->GetControllerManager()->createController(desc);

            _controllers[_controllerCount] = cController(id, controller, eyeHeight);
            _controllerCount += 1;

            return &_controllers[_controllerCount - 1];
        }

        cActor* mPhysics::AddActor(const std::string& id, const Category& staticOrDynamic, const Category& shapeType, const cSimulationScene* const scene, const cSubstance* const substance, const f32 mass, const render::sTransform* const transform)
        {
            glm::vec3 position = transform->Position;
            glm::vec3 scale = transform->Scale;

            PxTransform pose(PxVec3(position.y, position.x, position.z));
            
            PxShape* shape = nullptr;
            if (shapeType == Category::PHYSICS_SHAPE_PLANE)
                shape = _physics->createShape(PxPlaneGeometry(), *substance->GetSubstance());
            else if (shapeType == Category::PHYSICS_SHAPE_BOX)
                shape = _physics->createShape(PxBoxGeometry(scale.y, scale.x, scale.z), *substance->GetSubstance());

            PxActor* actor = nullptr;
            if (staticOrDynamic == Category::PHYSICS_ACTOR_STATIC)
            {
                actor = _physics->createRigidStatic(pose);
                ((PxRigidStatic*)actor)->attachShape(*shape);
            }
            else if (staticOrDynamic == Category::PHYSICS_ACTOR_DYNAMIC)
            {
                actor = _physics->createRigidDynamic(pose);
                ((PxRigidDynamic*)actor)->attachShape(*shape);
                PxRigidBodyExt::updateMassAndInertia(*((PxRigidBody*)actor), mass);
            }

            if (shape != nullptr)
                shape->release();

            if (actor != nullptr)
                scene->GetScene()->addActor(*actor);

            _actors[_actorCount] = cActor(id, actor);
            _actorCount += 1;

            return &_actors[_actorCount - 1];
        }

        void mPhysics::DeleteScene(const std::string& id)
        {
            for (usize i = 0; i < _sceneCount; i++)
            {
                if (_scenes[i].GetID() == id)
                {
                    _scenes[i].GetScene()->release();
                    _scenes[i].GetControllerManager()->release();

                    if (_sceneCount > 1)
                        _scenes[i] = _scenes[_sceneCount - 1];

                    return;
                }
            }
        }

        void mPhysics::DeleteSubstance(const std::string& id)
        {
            for (usize i = 0; i < _substanceCount; i++)
            {
                if (_substances[i].GetID() == id)
                {
                    _substances[i].GetSubstance()->release();

                    if (_substanceCount > 1)
                        _substances[i] = _substances[_substanceCount - 1];

                    return;
                }
            }
        }

        void mPhysics::DeleteActor(const std::string& id)
        {
            for (usize i = 0; i < _actorCount; i++)
            {
                if (_actors[i].GetID() == id)
                {
                    _actors[i].GetActor()->release();

                    if (_actorCount > 1)
                        _actors[i] = _actors[_actorCount - 1];

                    return;
                }
            }
        }

        void mPhysics::DeleteController(const std::string& id)
        {
            for (usize i = 0; i < _controllerCount; i++)
            {
                if (_controllers[i].GetID() == id)
                {
                    _controllers[i].GetController()->release();

                    if (_controllerCount > 1)
                        _controllers[i] = _controllers[_controllerCount - 1];

                    return;
                }
            }
        }

        void mPhysics::MoveController(const cController* const controller, const glm::vec3& position, const f32 minStep)
        {
            PxController* pxController = controller->GetController();
            f32 deltaTime = _app->GetDeltaTime();

            PxControllerFilters filters;
            PxU32 collisionFlags = pxController->move(
                PxVec3(position.y, position.x, position.z),
                minStep,
                deltaTime,
                filters
            );
        }

        glm::vec3 mPhysics::GetControllerPosition(const cController* const controller)
        {
            PxController* pxController = controller->GetController();
            PxExtendedVec3 position = pxController->getPosition();

            return glm::vec3(position.y, position.x + controller->GetEyeHeight(), position.z);
        }

        void mPhysics::Simulate()
        {
            for (usize i = 0; i < _sceneCount; i++)
            {
                auto scene = _scenes[i].GetScene();
                scene->simulate(1.0f / 60.0f);
                scene->fetchResults(true);
            }
        }

        cSimulationScene* mPhysics::GetScene(const std::string& id)
        {
            for (usize i = 0; i < _sceneCount; i++)
            {
                if (_scenes[i].GetID() == id)
                    return &_scenes[i];
            }

            return nullptr;
        }

        cController* mPhysics::GetController(const std::string& id)
        {
            for (usize i = 0; i < _controllerCount; i++)
            {
                if (_controllers[i].GetID() == id)
                    return &_controllers[i];
            }

            return nullptr;
        }

        /*void mPhysics::Update()
        {
            // Actors
            for (auto& pair : _actors)
            {
                sCTransform* transformComponent = pair.Scene->Get<sCTransform>(pair.Entity);
                sCPhysicsActor* actorComponent = pair.Scene->Get<sCPhysicsActor>(pair.Entity);

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
                    std::lock_guard<std::mutex> lock(_mutex);
                    transformComponent->Position = glm::vec3(actorTransform.p.y, actorTransform.p.x, actorTransform.p.z);
                    transformComponent->Rotation = glm::vec3(actorEuler.y, actorEuler.x, actorEuler.z);
                }
            }

            // Controllers
            for (auto& pair : _controllers)
            {
                sCTransform* transformComponent = pair.Scene->Get<sCTransform>(pair.Entity);
                sCPhysicsCharacterController* controllerComponent =
                    pair.Scene->Get<sCPhysicsCharacterController>(pair.Entity);

                if (controllerComponent->IsGravityEnabled == K_TRUE) {
                    SetCharacterControllerMovement(
                        pair, glm::vec3(0.0f, -controllerComponent->GravitySpeed, 0.0f)
                    );
                }

                PxExtendedVec3 position = controllerComponent->Controller->getPosition();
                
                {
                    std::lock_guard<std::mutex> lock(_mutex);
                    transformComponent->Position = glm::vec3(
                        position.y, position.x, position.z
                    );
                }
            }

            // Scenes
            for (auto& pair : _scenes)
            {
                sCPhysicsScene* component = pair.Scene->Get<sCPhysicsScene>(pair.Entity);
                component->Scene->simulate(1.0f / 60.0f);
                component->Scene->fetchResults(true);
            }
        }

        sCPhysicsScene* mPhysics::AddScene(const sEntityScenePair& scene)
        {
            PxSceneDesc sceneDesc(_physics->getTolerancesScale());
            sceneDesc.gravity = PxVec3(-9.81f, 0.0f, 0.0f);
            sceneDesc.cpuDispatcher = _cpuDispatcher;
            sceneDesc.simulationEventCallback = _simulationEvent;
            sceneDesc.filterShader = FilterShader;

            sCPhysicsScene* component = scene.Scene->Add<sCPhysicsScene>(scene.Entity);
            component->Scene = _physics->createScene(sceneDesc);
            component->ControllerManager = PxCreateControllerManager(*component->Scene);

            _scenes.push_back(scene);

            return component;
        }

        sCPhysicsActor* mPhysics::AddActor(
            const sEntityScenePair& scene,
            const sEntityScenePair& actor,
            const mPhysics::eActorDescriptor& actorDesc,
            const mPhysics::eShapeDescriptor& shapeDesc,
            const glm::vec4& extents,
            const sVertexBufferGeometry* const geometry
        )
        {
            sCTransform* transformComponent = actor.Scene->Get<sCTransform>(actor.Entity);

            glm::vec3& entityPosition = transformComponent->Position;
            glm::vec3 position = glm::vec3(entityPosition.y, entityPosition.x, entityPosition.z);

            // Creating material
            PxMaterial* physicsMaterial = _physics->createMaterial(0.05f, 0.05f, 0.05f);

            // Creation of actor
            PxActor* physicsActor = nullptr;
            
            switch (actorDesc)
            {

            case mPhysics::eActorDescriptor::STATIC:
                physicsActor = _physics->createRigidStatic(PxTransform(position.x, position.y, position.z));
                break;

            case mPhysics::eActorDescriptor::DYNAMIC:
                physicsActor = _physics->createRigidDynamic(PxTransform(position.x, position.y, position.z));
                ((PxRigidDynamic*)physicsActor)->setAngularDamping(0.75f);
                ((PxRigidDynamic*)physicsActor)->setLinearVelocity(PxVec3(0.0f, 0.0f, 0.0f));
                break;

            }

            if (physicsActor == nullptr) { return nullptr; }

            sCPhysicsScene* sceneComponent = scene.Scene->Get<sCPhysicsScene>(scene.Entity);
            sceneComponent->Scene->addActor(*physicsActor);

            // Shape creation
            PxShape* physicsShape = nullptr;

            switch (shapeDesc)
            {
                case mPhysics::eShapeDescriptor::SPHERE:
                {
                    physicsShape = _physics->createShape(
                        PxSphereGeometry(extents.x),
                        *physicsMaterial
                    );
                    break;
                }

                case mPhysics::eShapeDescriptor::CAPSULE:
                {
                    physicsShape = _physics->createShape(
                        PxCapsuleGeometry(extents.x, extents.y),
                        *physicsMaterial
                    );
                    break;
                }

                case mPhysics::eShapeDescriptor::BOX:
                {
                    physicsShape = _physics->createShape(
                        PxBoxGeometry(extents.x, extents.y, extents.z),
                        *physicsMaterial
                    );
                    break;
                }

                case mPhysics::eShapeDescriptor::PLANE:
                {
                    physicsShape = _physics->createShape(
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
                    for (usize i = 0; i < geometry->VertexCount; i++) {
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
                    PxTriangleMesh* triangleMesh = _physics->createTriangleMesh(readBuffer);
                    
                    ((PxRigidDynamic*)physicsActor)->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
                    
                    physicsShape = _physics->createShape(
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
            
            sCPhysicsActor* component = actor.Scene->Add<sCPhysicsActor>(actor.Entity);
            component->Actor = physicsActor;
            component->Material = physicsMaterial;

            _actors.push_back(actor);

            return component;
        }

        sCPhysicsCharacterController* mPhysics::AddCharacterController(
            const sEntityScenePair& scene,
            const sEntityScenePair& controller,
            const mPhysics::eShapeDescriptor& shapeDesc,
            const glm::vec4& extents
        )
        {
            sCPhysicsScene* sceneComponent = scene.Scene->Get<sCPhysicsScene>(scene.Entity);
            sCTransform* transformComponent = controller.Scene->Get<sCTransform>(controller.Entity);
            PxControllerManager* controllerManager = sceneComponent->ControllerManager;
            
            PxMaterial* physicsMaterial = _physics->createMaterial(0.05f, 0.05f, 0.05f);

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

            sCPhysicsCharacterController* controllerComponent =
                controller.Scene->Add<sCPhysicsCharacterController>(controller.Entity);
            controllerComponent->Controller = physicsController;
            controllerComponent->Material = physicsMaterial;

            _controllers.push_back(controller);
            
            return controllerComponent;
        }

        void mPhysics::SetForce(const sEntityScenePair& actor, const glm::vec3& force)
        {
            sCPhysicsActor* component = actor.Scene->Get<sCPhysicsActor>(actor.Entity);
            ((PxRigidDynamic*)component->Actor)->setForceAndTorque(
                PxVec3(force.y, force.x, force.z),
                PxVec3(0.0f, 0.0f, 0.0f)
            );
        }

        void mPhysics::SetCharacterControllerMovement(const sEntityScenePair& controller, const glm::vec3& direction)
        {
            sCPhysicsCharacterController* controllerComponent =
                controller.Scene->Get<sCPhysicsCharacterController>(controller.Entity);

            PxControllerFilters filters = PxControllerFilters();
            controllerComponent->Controller->move(
                PxVec3(direction.y, direction.x, direction.z),
                0.01f,
                1.0f,
                filters
            );
        }*/
    }
}