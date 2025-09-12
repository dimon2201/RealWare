#pragma once

#include <vector>
#include <string>
#include "../../thirdparty/glm/glm/glm.hpp"
#include "application.hpp"
#include "gameobject_features.hpp"
#include "types.hpp"

namespace realware
{
    namespace app
    {
        class cApplication;
    }

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
        class cController;
        class mPhysics;
    }

    namespace game
    {
        class cGameObject
        {
        public:
            explicit cGameObject();
            ~cGameObject() = default;
            
            inline app::cApplication* GetApplication() { return _app; }
            inline std::string GetID() const { return _id; }
            inline types::boolean GetVisible() const { return _isVisible; }
            inline types::boolean GetOpaque() const { return _isOpaque; }
            inline render::sVertexBufferGeometry* GetGeometry() const { return _geometry; }
            inline types::boolean GetIs2D() const { return _is2D; }
            glm::vec3 GetPosition() const;
            glm::vec3 GetRotation() const;
            glm::vec3 GetScale() const;
            inline glm::mat4 GetWorldMatrix() const { return _world; }
            inline glm::mat4 GetViewProjectionMatrix() const { return _viewProjection; }
            inline render::sTransform* GetTransform() const { return _transform; }
            inline render::cMaterial* GetMaterial() const { return _material; }
            inline font::cText* GetText() const { return _text; }
            inline render::sLight* GetLight() const { return _light; }
            inline physics::cActor* GetPhysicsActor() const { return _actor; }
            inline physics::cController* GetPhysicsController() const { return _controller; }

            inline void SetVisible(const types::boolean isVisible) { _isVisible = isVisible; }
            inline void SetOpaque(const types::boolean isOpaque) { _isOpaque = isOpaque; }
            inline void SetGeometry(const render::sVertexBufferGeometry* const geometry) { _geometry = (render::sVertexBufferGeometry*)geometry; }
            inline void SetIs2D(const types::boolean is2D) { _is2D = is2D; }
            void SetPosition(const glm::vec3& position);
            void SetRotation(const glm::vec3& rotation);
            void SetScale(const glm::vec3& scale);
            inline void SetWorldMatrix(const glm::mat4& world) { _world = world; }
            inline void SetViewProjectionMatrix(const glm::mat4& viewProjection) { _viewProjection = viewProjection; }
            inline void SetMaterial(const render::cMaterial* const material) { _material = (render::cMaterial*)material; }
            inline void SetText(const font::cText* const text) { _text = (font::cText*)text; }
            inline void SetLight(const render::sLight* const light) { _light = (render::sLight*)light; }
            void SetPhysicsActor(const GameObjectFeatures& staticOrDynamic, const GameObjectFeatures& shapeType, const physics::cSimulationScene* const scene, const physics::cSubstance* const substance, const types::f32 mass);
            void SetPhysicsController(const types::f32 eyeHeight, const types::f32 height, const types::f32 radius, const glm::vec3& up, const physics::cSimulationScene* const scene, const physics::cSubstance* const substance);

            friend class mGameObject;

        private:
            app::cApplication* _app = nullptr;
            types::boolean _isDeleted = types::K_TRUE;
            std::string _id = "";
            types::boolean _isVisible = types::K_TRUE;
            types::boolean _isOpaque = types::K_TRUE;
            render::sVertexBufferGeometry* _geometry = nullptr;
            types::boolean _is2D = types::K_FALSE;
            glm::mat4 _world = glm::mat4(1.0f);
            glm::mat4 _viewProjection = glm::mat4(1.0f);
            render::sTransform* _transform = nullptr;
            render::cMaterial* _material = nullptr;
            font::cText* _text = nullptr;
            render::sLight* _light = nullptr;
            physics::cActor* _actor = nullptr;
            physics::cController* _controller = nullptr;
        };

        class mGameObject
        {
        public:
            explicit mGameObject(const app::cApplication* const app) : _app((app::cApplication*)app), _maxGameObjectCount(((app::cApplication*)app)->GetDesc()->MaxGameObjectCount) {}
            ~mGameObject() = default;

            cGameObject* AddGameObject(const std::string& id);
            cGameObject* FindGameObject(const std::string& id);
            void DeleteGameObject(const std::string& id);

            inline std::vector<cGameObject>& GetObjects() { return _gameObjects; }

        private:
            app::cApplication* _app = nullptr;
            types::usize _maxGameObjectCount = 0;
            types::usize _gameObjectCount = 0;
            std::vector<cGameObject> _gameObjects = {};
        };
    }
}