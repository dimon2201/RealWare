#pragma once

#include "../../thirdparty/glm/glm/glm.hpp"
#include "render_manager.hpp"
#include "types.hpp"

namespace realware
{
    namespace core
    {
        class mCamera
        {
        public:
            enum eEulerAngle
            {
                PITCH = 0,
                YAW = 1,
                ROLL = 2
            };

            explicit mCamera(const cApplication* const app) : _app((cApplication*)app) {}
            ~mCamera() = default;

            void CreateCamera();
            void Update(const boolean updateMouseLook, const boolean updateMovement);
            void AddEuler(const eEulerAngle& angle, const f32 value);
            void Move(const f32 value);
            void Strafe(const f32 value);
            void Lift(const f32 value);

            inline cGameObject* GetCamera() const { return _camera; }
            inline glm::mat4 GetViewProjectionMatrix() const { return _viewProjection; }
            inline float GetMoveSpeed() const { return _moveSpeed; }

            inline void SetMoveSpeed(const f32 value) { _moveSpeed = value; }

        private:
            cApplication* _app = nullptr;
            cGameObject* _camera = nullptr;
            render::sTransform m_transform = {};
            glm::vec3 _euler = glm::vec3(0.0f);
            glm::vec3 _direction = glm::vec3(0.0f);
            glm::mat4 _view = glm::mat4(1.0f);
            glm::mat4 _projection = glm::mat4(1.0f);
            glm::mat4 _viewProjection = glm::mat4(1.0f);
            f32 _fov = 60.0f;
            f32 _zNear = 0.01f;
            f32 _zFar = 100.0f;
            f32 _moveSpeed = 0.1f;
            boolean _isMoving = K_FALSE;
            glm::vec2 _cursorPosition = glm::vec2(0.0f);
            glm::vec2 _prevCursorPosition = _cursorPosition;
        };
    }
}