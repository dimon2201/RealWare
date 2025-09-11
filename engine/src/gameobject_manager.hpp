#pragma once

#include <vector>
#include <string>
#include "../../thirdparty/glm/glm/glm.hpp"
#include "application.hpp"
#include "gameobject_features.hpp"
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

    namespace physics
    {
        class cSimulationScene;
        class cSubstance;
        class cActor;
        class mPhysics;
    }

    namespace core
    {
        class cGameObject
        {
        public:
            explicit cGameObject();
            ~cGameObject() = default;
            
            inline cApplication* GetApplication() { return _app; }
            inline std::string GetID() const { return _id; }
            inline boolean GetVisible() const { return _isVisible; }
            inline boolean GetOpaque() const { return _isOpaque; }
            inline render::sVertexBufferGeometry* GetGeometry() const { return _geometry; }
            inline boolean GetIs2D() const { return _is2D; }
            inline glm::vec3 GetPosition() const { return _position; }
            inline glm::vec3 GetRotation() const { return _rotation; }
            inline glm::vec3 GetScale() const { return _scale; }
            inline glm::mat4 GetWorldMatrix() const { return _world; }
            inline glm::mat4 GetViewProjectionMatrix() const { return _viewProjection; }
            inline render::cMaterial* GetMaterial() const { return _material; }
            inline font::cText* GetText() const { return _text; }
            inline render::sLight* GetLight() const { return _light; }

            inline void SetVisible(const boolean isVisible) { _isVisible = isVisible; }
            inline void SetOpaque(const boolean isOpaque) { _isOpaque = isOpaque; }
            inline void SetGeometry(const render::sVertexBufferGeometry* const geometry) { _geometry = (render::sVertexBufferGeometry*)geometry; }
            inline void SetIs2D(const boolean is2D) { _is2D = is2D; }
            inline void SetPosition(const glm::vec3& position) { _position = position; }
            inline void SetRotation(const glm::vec3& rotation) { _rotation = rotation; }
            inline void SetScale(const glm::vec3& scale) { _scale = scale; }
            inline void SetWorldMatrix(const glm::mat4& world) { _world = world; }
            inline void SetViewProjectionMatrix(const glm::mat4& viewProjection) { _viewProjection = viewProjection; }
            inline void SetMaterial(const render::cMaterial* const material) { _material = (render::cMaterial*)material; }
            inline void SetText(const font::cText* const text) { _text = (font::cText*)text; }
            inline void SetLight(const render::sLight* const light) { _light = (render::sLight*)light; }
            inline void SetPhysics(const GameObjectFeatures& staticOrDynamic, const GameObjectFeatures& shapeType, const physics::cSimulationScene* const scene, const physics::cSubstance* const substance, const f32 mass);

            friend class mGameObject;

        private:
            cApplication* _app = nullptr;
            boolean _isDeleted = K_TRUE;
            std::string _id = "";
            boolean _isVisible = K_TRUE;
            boolean _isOpaque = K_TRUE;
            render::sVertexBufferGeometry* _geometry = nullptr;
            boolean _is2D = K_FALSE;
            glm::vec3 _position = glm::vec3(0.0f);
            glm::vec3 _rotation = glm::vec3(0.0f);
            glm::vec3 _scale = glm::vec3(1.0f);
            glm::mat4 _world = glm::mat4(1.0f);
            glm::mat4 _viewProjection = glm::mat4(1.0f);
            render::sTransform* _transform = nullptr;
            render::cMaterial* _material = nullptr;
            font::cText* _text = nullptr;
            render::sLight* _light = nullptr;
            physics::cActor* _actor = nullptr;
        };

        class mGameObject
        {
        public:
            explicit mGameObject(const cApplication* const app) : _app((cApplication*)app), _maxGameObjectCount(((cApplication*)app)->GetDesc()->MaxGameObjectCount) {}
            ~mGameObject() = default;

            cGameObject* AddGameObject(const std::string& id);
            cGameObject* FindGameObject(const std::string& id);
            void DeleteGameObject(const std::string& id);

            inline std::vector<cGameObject>& GetObjects() { return _gameObjects; }

        private:
            cApplication* _app = nullptr;
            usize _maxGameObjectCount = 0;
            usize _gameObjectCount = 0;
            std::vector<cGameObject> _gameObjects = {};
        };
    }
}