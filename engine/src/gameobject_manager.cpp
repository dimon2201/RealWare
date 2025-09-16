#include "application.hpp"
#include "gameobject_manager.hpp"
#include "render_manager.hpp"
#include "physics_manager.hpp"
#include "memory_pool.hpp"

using namespace types;

namespace realware
{
    using namespace app;
    using namespace render;
    using namespace font;
    using namespace physics;
    using namespace utils;

    namespace game
    {
        cGameObject::cGameObject(const cMemoryPool* const memoryPool)
        {
            sTransform* pTransform = (sTransform*)(((cMemoryPool*)memoryPool)->Allocate(sizeof(sTransform)));
            _transform = new (pTransform) sTransform();
        }

        const glm::vec3& cGameObject::GetPosition() const
        {
            return _transform->Position;
        }

        const glm::vec3& cGameObject::GetRotation() const
        {
            return _transform->Rotation;
        }

        const glm::vec3& cGameObject::GetScale() const
        {
            return _transform->Scale;
        }

        void cGameObject::SetPosition(const glm::vec3& position)
        {
            _transform->Position = position;
        }

        void cGameObject::SetRotation(const glm::vec3& rotation)
        {
            _transform->Rotation = rotation;
        }

        void cGameObject::SetScale(const glm::vec3& scale)
        {
            _transform->Scale = scale;
        }

        void cGameObject::SetPhysicsActor(const Category& staticOrDynamic, const Category& shapeType, const sSimulationScene* const scene, const sSubstance* const substance, const f32 mass)
        {
            mPhysics* physics = App->GetPhysicsManager();
            _actor = physics->AddActor(
                GetID(),
                staticOrDynamic,
                shapeType,
                scene,
                substance,
                mass,
                _transform
            );
        }

        void cGameObject::SetPhysicsController(const f32 eyeHeight, const f32 height, const f32 radius, const glm::vec3& up, const sSimulationScene* const scene, const sSubstance* const substance)
        {
            mPhysics* physics = App->GetPhysicsManager();

            _controller = physics->AddController(
                GetID(),
                eyeHeight,
                height,
                radius,
                _transform,
                up,
                scene,
                substance
            );
        }

        mGameObject::mGameObject(const cApplication* const app) :
            _app((cApplication*)app), _maxGameObjectCount(((cApplication*)app)->GetDesc()->MaxGameObjectCount), _gameObjects((cApplication*)_app, _maxGameObjectCount)
        {
        }

        cGameObject* mGameObject::AddGameObject(const std::string& id)
        {
            return _gameObjects.Add(id, _app->GetMemoryPool());
        }

        cGameObject* mGameObject::FindGameObject(const std::string& id)
        {
            return _gameObjects.Find(id);
        }

        void mGameObject::DeleteGameObject(const std::string& id)
        {
            _gameObjects.Delete(id);
        }
    }
}