#pragma once

#include <vector>
#include <string>
#include "../../thirdparty/glm/glm/glm.hpp"
#include "types.hpp"

namespace realware
{
    namespace render
    {
        struct sVertexBufferGeometry;
        struct sLight;
        struct sTransform;
        class cMaterial;
    }

    namespace font
    {
        class cText;
    }

    using namespace render;
    using namespace font;

    namespace core
    {
        class cApplication;
        class mGameObject;
        struct sArea;

        class cGameObject
        {
        public:
            friend class mGameObject;

            explicit cGameObject();
            ~cGameObject() = default;

            inline void SetVisible(boolean isVisible) { m_isVisible = isVisible; }
            inline void SetOpaque(boolean isOpaque) { m_isOpaque = isOpaque; }
            inline void SetGeometry(sVertexBufferGeometry* geometry) { m_geometry = geometry; }
            inline void SetIs2D(boolean is2D) { m_is2D = is2D; }
            inline void SetPosition(const glm::vec3& position) { m_position = position; }
            inline void SetRotation(const glm::vec3& rotation) { m_rotation = rotation; }
            inline void SetScale(const glm::vec3& scale) { m_scale = scale; }
            inline void SetWorldMatrix(const glm::mat4& world) { m_world = world; }
            inline void SetViewProjectionMatrix(const glm::mat4& viewProjection) { m_viewProjection = viewProjection; }
            inline void SetMaterial(cMaterial* material) { m_material = material; }
            inline void SetText(font::cText* text) { m_text = text; }
            inline void SetLight(render::sLight* light) { m_light = light; }
            inline std::string GetID() const { return m_id; }
            inline boolean GetVisible() { return m_isVisible; }
            inline boolean GetOpaque() { return m_isOpaque; }
            inline sVertexBufferGeometry* GetGeometry() { return m_geometry; }
            inline boolean GetIs2D() { return m_is2D; }
            inline glm::vec3 GetPosition() { return m_position; }
            inline glm::vec3 GetRotation() { return m_rotation; }
            inline glm::vec3 GetScale() { return m_scale; }
            inline glm::mat4 GetWorldMatrix() { return m_world; }
            inline glm::mat4 GetViewProjectionMatrix() const { return m_viewProjection; }
            inline cMaterial* GetMaterial() { return m_material; }
            inline font::cText* GetText() { return m_text; }
            inline render::sLight* GetLight() { return m_light; }

        private:
            boolean m_isDeleted = K_TRUE;
            std::string m_id = "";
            boolean m_isVisible = K_TRUE;
            boolean m_isOpaque = K_TRUE;
            sVertexBufferGeometry* m_geometry = nullptr;
            boolean m_is2D = K_FALSE;
            glm::vec3 m_position = glm::vec3(0.0f);
            glm::vec3 m_rotation = glm::vec3(0.0f);
            glm::vec3 m_scale = glm::vec3(1.0f);
            glm::mat4 m_world = glm::mat4(1.0f);
            glm::mat4 m_viewProjection = glm::mat4(1.0f);
            sTransform* m_transform = nullptr;
            cMaterial* m_material = nullptr;
            cText* m_text = nullptr;
            sLight* m_light = nullptr;
        };

        class mGameObject
        {
        public:
            explicit mGameObject(cApplication* app);
            ~mGameObject() {}

            cGameObject* CreateGameObject(const std::string& id);
            cGameObject* FindGameObject(const std::string& id);
            void DeleteGameObject(const std::string& id);

            inline std::vector<cGameObject>& GetObjects() { return m_gameObjects; }

        private:
            cApplication* m_app;
            usize m_maxGameObjectCount = 0;
            usize m_gameObjectCount = 0;
            std::vector<cGameObject> m_gameObjects;
        };
    }
}