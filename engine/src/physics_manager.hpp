#pragma once

#include <iostream>
#include <vector>
#include <mutex>
#include <PxPhysics.h>
#include <PxPhysicsAPI.h>
#include "../../thirdparty/glm/glm/glm.hpp"
#include "ecs.hpp"

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

            mPhysics(cApplication* app);
            ~mPhysics();

            void Update();
            core::sCPhysicsScene* AddScene(const core::sEntityScenePair& scene);
            core::sCPhysicsActor* AddActor(
                const core::sEntityScenePair& scene,
                const core::sEntityScenePair& actor,
                mPhysics::eActorDescriptor actorDesc,
                mPhysics::eShapeDescriptor shapeDesc,
                const glm::vec4& extents,
                render::sVertexBufferGeometry* geometry
            );
            core::sCPhysicsCharacterController* AddCharacterController(
                const core::sEntityScenePair& scene,
                const core::sEntityScenePair& controller,
                mPhysics::eShapeDescriptor shapeDesc,
                const glm::vec4& extents
            );
            
            void SetForce(const core::sEntityScenePair& actor, const glm::vec3& force);
            void SetCharacterControllerMovement(
                const core::sEntityScenePair& controller,
                const glm::vec3& direction
            );

        private:
            cApplication* m_app = nullptr;
            cAllocator* m_allocator = nullptr;
            cError* m_error = nullptr;
            cCPUDispatcher* m_cpuDispatcher = nullptr;
            cSimulationEvent* m_simulationEvent = nullptr;
            physx::PxFoundation* m_foundation = nullptr;
            physx::PxPhysics* m_physics = nullptr;
            std::vector<core::sEntityScenePair> m_scenes = {};
            std::vector<core::sEntityScenePair> m_actors = {};
            std::vector<core::sEntityScenePair> m_controllers = {};
            std::mutex m_mutex;
        };
    }
}