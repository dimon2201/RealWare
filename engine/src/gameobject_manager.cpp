#include "gameobject_manager.hpp"
#include "render_manager.hpp"

namespace realware
{
    namespace core
    {
        mGameObject::mGameObject(cApplication* app, usize maxObjectCount)
        {
            m_app = app;
            m_maxGameObjectCount = maxObjectCount;
            m_gameObjects.resize(m_maxGameObjectCount);
        }

        cGameObject::cGameObject()
        {
            m_transform = new sTransform();
        }

        cGameObject* mGameObject::CreateGameObject(const std::string& id)
        {
            for (usize i = 0; i < m_maxGameObjectCount; i++)
            {
                if (m_gameObjects[i].m_isDeleted == K_TRUE)
                {
                    m_gameObjects[i] = cGameObject();
                    m_gameObjects[i].m_isDeleted = K_FALSE;
                    m_gameObjects[i].m_id = id;
                    return &m_gameObjects[i];
                }
            }

            return nullptr;
        }

        cGameObject* mGameObject::FindGameObject(const std::string& id)
        {
            for (usize i = 0; i < m_maxGameObjectCount; i++)
            {
                if (m_gameObjects[i].m_isDeleted == K_FALSE && m_gameObjects[i].GetID() == id)
                {
                    return &m_gameObjects[i];
                }
            }
            
            return nullptr;
        }

        void mGameObject::DeleteGameObject(const std::string& id)
        {
            cGameObject* object = FindGameObject(id);
            object->m_isDeleted = K_TRUE;
            delete object->m_transform;
        }
    }
}