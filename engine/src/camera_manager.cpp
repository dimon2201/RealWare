#include "../../thirdparty/glm/glm/gtc/matrix_transform.hpp"
#include "../../thirdparty/glm/glm/gtc/quaternion.hpp"
#include "../../thirdparty/glm/glm/gtx/quaternion.hpp"
#include "camera_manager.hpp"
#include "physics_manager.hpp"
#include "render_manager.hpp"
#include "render_context.hpp"
#include "application.hpp"
#include "gameobject_manager.hpp"

namespace realware
{
    using namespace app;
    using namespace render;
    using namespace physics;
    using namespace types;

    namespace game
    {
        void mCamera::CreateCamera()
        {
            mGameObject* gameObjectManager = _app->GetGameObjectManager();
            _cameraGameObject = gameObjectManager->AddGameObject(K_CAMERA_ID);
        }

        void mCamera::DestroyCamera()
        {
            mGameObject* gameObjectManager = _app->GetGameObjectManager();
            gameObjectManager->DeleteGameObject(K_CAMERA_ID);
        }

        void mCamera::Update(const types::boolean updateMouseLook, const types::boolean updateMovement)
        {
            const f32 deltaTime = _app->GetDeltaTime();

            if (_euler.x > glm::radians(65.0f))
                _euler.x = glm::radians(65.0f);
            else if (_euler.x < glm::radians(-65.0f))
                _euler.x = glm::radians(-65.0f);

            const glm::quat quatX = glm::angleAxis(_euler.x, glm::vec3(1.0f, 0.0f, 0.0f));
            const glm::quat quatY = glm::angleAxis(_euler.y, glm::vec3(0.0f, 1.0f, 0.0f));
            const glm::quat quatZ = glm::angleAxis(_euler.z, glm::vec3(0.0f, 0.0f, 1.0f));
            _direction = quatZ * quatY * quatX * glm::vec3(0.0f, 0.0f, -1.0f);

            sTransform* const transform = _cameraGameObject->GetTransform();
            _view = glm::lookAtRH(transform->Position, transform->Position + _direction, glm::vec3(0.0f, 1.0f, 0.0f));
            _projection = glm::perspective(glm::radians(_fov), (float)_app->GetWindowSize().x / (float)_app->GetWindowSize().y, _zNear, _zFar);
            _viewProjection = _projection * _view;

            if (updateMouseLook == K_TRUE)
            {
                double x = 0.0, y = 0.0;
                glfwGetCursorPos((GLFWwindow*)_app->GetWindow(), &x, &y);

                _prevCursorPosition = _cursorPosition;
                _cursorPosition = glm::vec2(x, y);

                glm::vec2 mouseDelta = _prevCursorPosition - _cursorPosition;
                AddEuler(mCamera::eEulerAngle::PITCH, mouseDelta.y * _mouseSensitivity * deltaTime);
                AddEuler(mCamera::eEulerAngle::YAW, mouseDelta.x * _mouseSensitivity * deltaTime);
            }
            if (updateMovement == K_TRUE)
            {
                f32 forward = _app->GetKey('W') * _moveSpeed * deltaTime;
                f32 backward = _app->GetKey('S') * _moveSpeed * deltaTime;
                f32 left = _app->GetKey('A') * _moveSpeed * deltaTime;
                f32 right = _app->GetKey('D') * _moveSpeed * deltaTime;
                if (forward > 0.0f || backward > 0.0f || left > 0.0f || right > 0.0f)
                {
                    Move(forward);
                    Move(-backward);
                    Strafe(-left);
                    Strafe(right);

                    _isMoving = K_TRUE;
                }
                else
                {
                    _isMoving = K_FALSE;
                }
            }

            _cameraGameObject->SetViewProjectionMatrix(_viewProjection);
        }

        void mCamera::AddEuler(const mCamera::eEulerAngle& angle, const f32 value)
        {
            _euler[(int)angle] += value;
        }

        void mCamera::Move(const f32 value)
        {
            mPhysics* const physics = _app->GetPhysicsManager();
            cController* const controller = _cameraGameObject->GetPhysicsController();
            sTransform* const transform = _cameraGameObject->GetTransform();
            const glm::vec3 position = transform->Position;
            const glm::vec3 newPosition = transform->Position + _direction * value;
            
            physics->MoveController(
                controller,
                newPosition - position
            );
            const glm::vec3 cameraPosition = physics->GetControllerPosition(controller);

            _cameraGameObject->SetPosition(cameraPosition);
        }

        void mCamera::Strafe(const f32 value)
        {
            mPhysics* const physics = _app->GetPhysicsManager();
            cController* const controller = _cameraGameObject->GetPhysicsController();
            sTransform* const transform = _cameraGameObject->GetTransform();
            const glm::vec3 right = glm::cross(_direction, glm::vec3(0.0f, 1.0f, 0.0f));
            const glm::vec3 position = transform->Position;
            const glm::vec3 newPosition = transform->Position + right * value;

            physics->MoveController(
                controller,
                newPosition - position
            );
            const glm::vec3 cameraPosition = physics->GetControllerPosition(controller);

            _cameraGameObject->SetPosition(cameraPosition);
        }

        void mCamera::Lift(const f32 value)
        {
            mPhysics* const physics = _app->GetPhysicsManager();
            cController* const controller = _cameraGameObject->GetPhysicsController();
            sTransform* const transform = _cameraGameObject->GetTransform();
            const glm::vec3 position = transform->Position;
            const glm::vec3 newPosition = transform->Position + glm::vec3(0.0f, 1.0f, 0.0f) * value;

            physics->MoveController(
                controller,
                newPosition - position
            );
            const glm::vec3 cameraPosition = physics->GetControllerPosition(controller);

            _cameraGameObject->SetPosition(cameraPosition);
        }
    }
}