#pragma once

#include <iostream>
#include <vector>
#include <mutex>
#include <PxPhysics.h>
#include <PxPhysicsAPI.h>
#include "../../thirdparty/glm/glm/glm.hpp"

namespace physx
{
    class PxActor;
    class PxRigidDynamic;
    class PxShape;
    class PxMaterial;
    class PxJoint;
    class PxScene;
    class PxFoundation;
    class PxPhysics;
}

namespace realware
{
    namespace core
    {
        class cApplication;
    }

    namespace render
    {
        struct sVertexBufferGeometry;
    }

    using namespace core;
    using namespace render;

    namespace physics
    {
        class cAllocator : public physx::PxAllocatorCallback
        {
            virtual void* allocate(size_t size, const char* typeName, const char* filename, int line) override final
            {
                return malloc(size);
            }

            virtual void deallocate(void* ptr) override final
            {
                free(ptr);
            }
        };

        class cError : public physx::PxErrorCallback
        {
            virtual void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line) override final
            {
                std::cout << message << std::endl;
            }
        };

        class cCPUDispatcher : public physx::PxCpuDispatcher
        {
            virtual void submitTask(physx::PxBaseTask& task) override final
            {
                task.run();
                task.release();
            }

            virtual uint32_t getWorkerCount() const override final
            {
                return 0;
            }
        };

        class cSimulationEvent : public physx::PxSimulationEventCallback
        {
			virtual void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) override final {}
			virtual void onWake(physx::PxActor** actors, physx::PxU32 count) override final {}
			virtual void onSleep(physx::PxActor** actors, physx::PxU32 count) override final {}
			virtual void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) override final {}
			virtual void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) override final {}
			virtual void onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count) override final {}
        };

        class mPhysics
        {
        public:
            enum class eActorDescriptor
            {
                STATIC = 0,
                DYNAMIC = 1
            };

            enum class eShapeDescriptor
            {
                SPHERE = 0,
                CAPSULE = 1,
                BOX = 2,
                PLANE = 3,
                TRIANGLE_MESH = 4
            };

            explicit mPhysics(const cApplication* const app);
            ~mPhysics();

            void Update();
            /*sCPhysicsScene* AddScene(const sEntityScenePair& scene);
            sCPhysicsActor* AddActor(
                const sEntityScenePair& scene,
                const sEntityScenePair& actor,
                const mPhysics::eActorDescriptor& actorDesc,
                const mPhysics::eShapeDescriptor& shapeDesc,
                const glm::vec4& extents,
                const sVertexBufferGeometry* const geometry
            );
            sCPhysicsCharacterController* AddCharacterController(
                const sEntityScenePair& scene,
                const sEntityScenePair& controller,
                const mPhysics::eShapeDescriptor& shapeDesc,
                const glm::vec4& extents
            );
            
            void SetForce(const core::sEntityScenePair& actor, const glm::vec3& force);
            void SetCharacterControllerMovement(const core::sEntityScenePair& controller, const glm::vec3& direction);*/

        private:
            cApplication* _app = nullptr;
            cAllocator* _allocator = nullptr;
            cError* _error = nullptr;
            cCPUDispatcher* _cpuDispatcher = nullptr;
            cSimulationEvent* _simulationEvent = nullptr;
            physx::PxFoundation* _foundation = nullptr;
            physx::PxPhysics* _physics = nullptr;
            //std::vector<core::sEntityScenePair> _scenes = {};
            //std::vector<core::sEntityScenePair> _actors = {};
            //std::vector<core::sEntityScenePair> _controllers = {};
            std::mutex _mutex;
        };
    }
}