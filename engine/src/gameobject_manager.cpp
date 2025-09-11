#include "application.hpp"
#include "gameobject_manager.hpp"
#include "render_manager.hpp"
#include "physics_manager.hpp"

namespace realware
{
    using namespace render;
    using namespace font;
    using namespace physics;

    namespace core
    {
        cGameObject::cGameObject()
        {
            _transform = new sTransform();
        }

        void cGameObject::SetPhysics(const GameObjectFeatures& staticOrDynamic, const GameObjectFeatures& shapeType, const cSimulationScene* const scene, const cSubstance* const substance, const f32 mass)
        {
            mPhysics* physics = _app->GetPhysicsManager();
            _actor = physics->AddActor(
                _id,
                staticOrDynamic,
                shapeType,
                scene,
                substance,
                mass,
                _transform
            );
        }

        cGameObject* mGameObject::AddGameObject(const std::string& id)
        {
            const usize gameObjectCount = _gameObjects.size();

            for (usize i = 0; i < gameObjectCount; i++)
            {
                if (_gameObjects[i]._isDeleted == K_TRUE)
                {
                    _gameObjects[i] = cGameObject();
                    _gameObjects[i]._app = _app;
                    _gameObjects[i]._isDeleted = K_FALSE;
                    _gameObjects[i]._id = id;

                    return &_gameObjects[i];
                }
            }
            
            if (gameObjectCount < _maxGameObjectCount)
            {
                cGameObject gameObject = cGameObject();
                gameObject._id = id;
                gameObject._isDeleted = K_FALSE;
                _gameObjects.push_back(gameObject);

                return &_gameObjects[gameObjectCount];
            }

            return nullptr;
        }

        cGameObject* mGameObject::FindGameObject(const std::string& id)
        {
            for (usize i = 0; i < _maxGameObjectCount; i++)
            {
                if (_gameObjects[i]._isDeleted == K_FALSE && _gameObjects[i].GetID() == id)
                    return &_gameObjects[i];
            }
            
            return nullptr;
        }

        void mGameObject::DeleteGameObject(const std::string& id)
        {
            cGameObject* object = FindGameObject(id);
            if (object != nullptr)
            {
                object->_isDeleted = K_TRUE;
                delete object->_transform;
            }
        }
    }
}