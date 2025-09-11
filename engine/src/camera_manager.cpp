#include "../../thirdparty/glm/glm/gtc/matrix_transform.hpp"
#include "../../thirdparty/glm/glm/gtc/quaternion.hpp"
#include "../../thirdparty/glm/glm/gtx/quaternion.hpp"
#include "camera_manager.hpp"
//#include "physics_manager.hpp"
#include "render_context.hpp"
#include "application.hpp"
#include "gameobject_manager.hpp"

namespace realware
{
    namespace core
    {
        void mCamera::CreateCamera()
        {
            mGameObject* gameObjectManager = _app->GetGameObjectManager();
            _camera = gameObjectManager->AddGameObject(K_CAMERA_ID);
        }

        void mCamera::DestroyCamera()
        {
            mGameObject* gameObjectManager = _app->GetGameObjectManager();
            gameObjectManager->DeleteGameObject(K_CAMERA_ID);
        }

        void mCamera::Update(const boolean updateMouseLook, const boolean updateMovement)
        {
            if (_euler.x > glm::radians(65.0f))
                _euler.x = glm::radians(65.0f);
            else if (_euler.x < glm::radians(-65.0f))
                _euler.x = glm::radians(-65.0f);

            const glm::quat quatX = glm::angleAxis(_euler.x, glm::vec3(1.0f, 0.0f, 0.0f));
            const glm::quat quatY = glm::angleAxis(_euler.y, glm::vec3(0.0f, 1.0f, 0.0f));
            const glm::quat quatZ = glm::angleAxis(_euler.z, glm::vec3(0.0f, 0.0f, 1.0f));
            _direction = quatZ * quatY * quatX * glm::vec3(0.0f, 0.0f, -1.0f);

            _view = glm::lookAtRH(m_transform.Position, m_transform.Position + _direction, glm::vec3(0.0f, 1.0f, 0.0f));
            _projection = glm::perspective(glm::radians(_fov), (float)_app->GetWindowSize().x / (float)_app->GetWindowSize().y, _zNear, _zFar);
            _viewProjection = _projection * _view;

            if (updateMouseLook == K_TRUE)
            {
                double x = 0.0, y = 0.0;
                glfwGetCursorPos((GLFWwindow*)_app->GetWindow(), &x, &y);

                _prevCursorPosition = _cursorPosition;
                _cursorPosition = glm::vec2(x, y);

                glm::vec2 mouseDelta = _prevCursorPosition - _cursorPosition;
                AddEuler(mCamera::eEulerAngle::PITCH, mouseDelta.y * 0.01f);
                AddEuler(mCamera::eEulerAngle::YAW, mouseDelta.x * 0.01f);
            }
            if (updateMovement == K_TRUE)
            {
                float forward = _app->GetKey('W') * _moveSpeed;
                float backward = _app->GetKey('S') * _moveSpeed;
                float left = _app->GetKey('A') * _moveSpeed;
                float right = _app->GetKey('D') * _moveSpeed;
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

            _camera->SetViewProjectionMatrix(_viewProjection);
        }

        void mCamera::AddEuler(const mCamera::eEulerAngle& angle, const f32 value)
        {
            _euler[(int)angle] += value;
        }

        void mCamera::Move(const f32 value)
        {
            m_transform.Position += _direction * value;

            /*sCPhysicsCharacterController* controller =
                scene->Get<sCPhysicsCharacterController>(camera->Owner);
            if (controller != nullptr)
            {
                m_app->GetPhysicsManager()->SetCharacterControllerMovement(
                    { camera->Owner, scene },
                    camera->Direction * value
                );
            }*/
        }

        void mCamera::Strafe(const f32 value)
        {
            glm::vec3 right = glm::cross(_direction, glm::vec3(0.0f, 1.0f, 0.0f));
            m_transform.Position += right * value;

            /*sCPhysicsCharacterController* controller =
                scene->Get<sCPhysicsCharacterController>(camera->Owner);
            if (controller != nullptr)
            {
                m_app->GetPhysicsManager()->SetCharacterControllerMovement(
                    { camera->Owner, scene },
                    right * value
                );
            }*/
        }

        void mCamera::Lift(const f32 value)
        {
            m_transform.Position.y += value;

            /*sCPhysicsCharacterController* controller =
                scene->Get<sCPhysicsCharacterController>(camera->Owner);
            if (controller != nullptr)
            {
                m_app->GetPhysicsManager()->SetCharacterControllerMovement(
                    { camera->Owner, scene },
                    glm::vec3(0.0f, value, 0.0f)
                );
            }*/
        }
    }
}