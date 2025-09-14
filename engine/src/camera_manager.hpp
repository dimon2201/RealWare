#pragma once

#include "../../thirdparty/glm/glm/glm.hpp"
#include "category.hpp"
#include "types.hpp"

namespace realware
{
    namespace app
    {
        class cApplication;
    }

    namespace game
    {
        class cGameObject;

        class mCamera
        {
        public:
            explicit mCamera(const app::cApplication* const app) : _app((app::cApplication*)app) {}
            ~mCamera() = default;

            void CreateCamera();
            void DestroyCamera();
            void Update(const types::boolean updateMouseLook, const types::boolean updateMovement);
            void AddEuler(const game::Category& angle, const types::f32 value);
            void Move(const types::f32 value);
            void Strafe(const types::f32 value);
            void Lift(const types::f32 value);

            inline cGameObject* GetCameraGameObject() const { return _cameraGameObject; }
            inline glm::mat4 GetViewProjectionMatrix() const { return _viewProjection; }
            inline float GetMouseSensitivity() const { return _mouseSensitivity; }
            inline float GetMoveSpeed() const { return _moveSpeed; }

            inline void SetMouseSensitivity(const types::f32 value) { _mouseSensitivity = value; }
            inline void SetMoveSpeed(const types::f32 value) { _moveSpeed = value; }

        private:
            static constexpr const char* K_CAMERA_ID = "__Camera";

            app::cApplication* _app = nullptr;
            cGameObject* _cameraGameObject = nullptr;
            glm::vec3 _euler = glm::vec3(0.0f);
            glm::vec3 _direction = glm::vec3(0.0f);
            glm::mat4 _view = glm::mat4(1.0f);
            glm::mat4 _projection = glm::mat4(1.0f);
            glm::mat4 _viewProjection = glm::mat4(1.0f);
            types::f32 _fov = 60.0f;
            types::f32 _zNear = 0.01f;
            types::f32 _zFar = 100.0f;
            types::f32 _mouseSensitivity = 1.0f;
            types::f32 _moveSpeed = 1.0f;
            types::boolean _isMoving = types::K_FALSE;
            glm::vec2 _cursorPosition = glm::vec2(0.0f);
            glm::vec2 _prevCursorPosition = _cursorPosition;
        };
    }
}