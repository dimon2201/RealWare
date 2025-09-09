#include "application.hpp"
#include "gameobject_manager.hpp"
#include "render_manager.hpp"

namespace realware
{
    namespace core
    {
        mGameObject::mGameObject(cApplication* app)
        {
            m_app = app;
            m_maxGameObjectCount = app->GetDesc()->MaxGameObjectCount;
        }

        cGameObject::cGameObject()
        {
            m_transform = new sTransform();
        }

        cGameObject* mGameObject::CreateGameObject(const std::string& id)
        {
            const usize gameObjectCount = m_gameObjects.size();

            for (usize i = 0; i < gameObjectCount; i++)
            {
                if (m_gameObjects[i].m_isDeleted == K_TRUE)
                {
                    m_gameObjects[i] = cGameObject();
                    m_gameObjects[i].m_isDeleted = K_FALSE;
                    m_gameObjects[i].m_id = id;

                    return &m_gameObjects[i];
                }
            }
            
            if (gameObjectCount < m_maxGameObjectCount)
            {
                cGameObject gameObject = cGameObject();
                gameObject.m_id = id;
                gameObject.m_isDeleted = K_FALSE;
                m_gameObjects.push_back(gameObject);

                return &m_gameObjects[gameObjectCount];
            }

            return nullptr;
        }

        cGameObject* mGameObject::FindGameObject(const std::string& id)
        {
            for (usize i = 0; i < m_maxGameObjectCount; i++)
            {
                if (m_gameObjects[i].m_isDeleted == K_FALSE && m_gameObjects[i].GetID() == id)
                    return &m_gameObjects[i];
            }
            
            return nullptr;
        }

        void mGameObject::DeleteGameObject(const std::string& id)
        {
            cGameObject* object = FindGameObject(id);
            if (object != nullptr)
            {
                object->m_isDeleted = K_TRUE;
                delete object->m_transform;
            }
        }
    }
}