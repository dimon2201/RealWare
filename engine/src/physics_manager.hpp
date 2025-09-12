#pragma once

#include <iostream>
#include <vector>
#include <mutex>
#include <PxPhysics.h>
#include <PxPhysicsAPI.h>
#include "../../thirdparty/glm/glm/glm.hpp"
#include "category.hpp"

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
    namespace app
    {
        class cApplication;
    }

    namespace render
    {
        struct sVertexBufferGeometry;
        struct sTransform;
    }

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

        class cSimulationScene
        {
        public:
            cSimulationScene() = default;
            explicit cSimulationScene(const std::string& id, const physx::PxScene* const scene, const physx::PxControllerManager* const controllerManager) : _id(id), _scene((physx::PxScene*)scene), _controllerManager((physx::PxControllerManager*)controllerManager) {}
            ~cSimulationScene() = default;

            inline const std::string& GetID() const { return _id; }
            inline physx::PxScene* GetScene() const { return _scene; }
            inline physx::PxControllerManager* GetControllerManager() const { return _controllerManager; }

        private:
            std::string _id = "";
            physx::PxScene* _scene = nullptr;
            physx::PxControllerManager* _controllerManager = nullptr;
        };

        class cSubstance
        {
        public:
            cSubstance() = default;
            explicit cSubstance(const std::string& id, const physx::PxMaterial* const substance) : _id(id), _substance((physx::PxMaterial*)substance) {}
            ~cSubstance() = default;

            inline const std::string& GetID() const { return _id; }
            inline physx::PxMaterial* GetSubstance() const { return _substance; }

        private:
            std::string _id = "";
            physx::PxMaterial* _substance = nullptr;
        };

        class cController
        {
        public:
            cController() = default;
            explicit cController(const std::string& id, const physx::PxController* const controller, const types::f32 eyeHeight) : _id(id), _controller((physx::PxController*)controller), _eyeHeight(eyeHeight) {}
            ~cController() = default;

            inline const std::string& GetID() const { return _id; }
            inline physx::PxController* GetController() const { return _controller; }
            inline types::f32 GetEyeHeight() const { return _eyeHeight; }

        private:
            std::string _id = "";
            physx::PxController* _controller = nullptr;
            types::f32 _eyeHeight = 0.0f;
        };

        class cActor
        {
        public:
            enum class Type
            {
                STATIC = 0,
                DYNAMIC = 1
            };

            enum class Shape
            {
                BOX = 0
            };

            cActor() = default;
            explicit cActor(const std::string& id, const physx::PxActor* const actor) : _id(id), _actor((physx::PxActor*)actor) {}
            ~cActor() = default;

            inline const std::string& GetID() const { return _id; }
            inline physx::PxActor* GetActor() const { return _actor; }

        private:
            std::string _id = "";
            physx::PxActor* _actor = nullptr;
        };

        class mPhysics
        {
        public:
            explicit mPhysics(const app::cApplication* const app);
            ~mPhysics();

            cSimulationScene* AddScene(const std::string& id, const glm::vec3& gravity = glm::vec3(0.0f, -9.81f, 0.0f));
            cSubstance* AddSubstance(const std::string& id, const glm::vec3& params = glm::vec3(0.5f, 0.5f, 0.6f));
            cActor* AddActor(const std::string& id, const game::Category& staticOrDynamic, const game::Category& shapeType, const cSimulationScene* const scene, const cSubstance* const substance, const types::f32 mass, const render::sTransform* const transform);
            cController* AddController(const std::string& id, const types::f32 eyeHeight, const types::f32 height, const types::f32 radius, const render::sTransform* const transform, const glm::vec3& up, const cSimulationScene* const scene, const cSubstance* const substance);
            void DeleteScene(const std::string& id);
            void DeleteSubstance(const std::string& id);
            void DeleteActor(const std::string& id);
            void DeleteController(const std::string& id);

            void MoveController(const cController* const controller, const glm::vec3& position, const types::f32 minStep = 0.001f);
            glm::vec3 GetControllerPosition(const cController* const controller);

            void Simulate();

            cSimulationScene* GetScene(const std::string&);
            cController* GetController(const std::string&);

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
            app::cApplication* _app = nullptr;
            cAllocator* _allocator = nullptr;
            cError* _error = nullptr;
            cCPUDispatcher* _cpuDispatcher = nullptr;
            cSimulationEvent* _simulationEvent = nullptr;
            physx::PxFoundation* _foundation = nullptr;
            physx::PxPhysics* _physics = nullptr;
            std::mutex _mutex;
            types::usize _sceneCount = 0;
            types::usize _substanceCount = 0;
            types::usize _actorCount = 0;
            types::usize _controllerCount = 0;
            std::vector<cSimulationScene> _scenes = {};
            std::vector<cSubstance> _substances = {};
            std::vector<cActor> _actors = {};
            std::vector<cController> _controllers = {};
        };
    }
}