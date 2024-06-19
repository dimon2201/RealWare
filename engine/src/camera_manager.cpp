#include "../../thirdparty/glm/glm/gtc/matrix_transform.hpp"
#include "../../thirdparty/glm/glm/gtc/quaternion.hpp"
#include "../../thirdparty/glm/glm/gtx/quaternion.hpp"
#include "camera_manager.hpp"
#include "render_manager.hpp"
#include "physics_manager.hpp"
#include "render_context.hpp"
#include "application.hpp"

namespace realware
{
    namespace core
    {
        mCamera::mCamera(cApplication* app)
        {
            m_app = app;
            m_transform = new render::cTransform();
        }

        mCamera::~mCamera()
        {
            delete m_transform;
        }

        void mCamera::Update(boolean updateMouseLook, boolean updateMovement)
        {
            if (m_euler.x > glm::radians(65.0f))
            {
                m_euler.x = glm::radians(65.0f);
            }
            else if (m_euler.x < glm::radians(-65.0f))
            {
                m_euler.x = glm::radians(-65.0f);
            }

            glm::quat quatX = glm::angleAxis(m_euler.x, glm::vec3(1.0f, 0.0f, 0.0f));
            glm::quat quatY = glm::angleAxis(m_euler.y, glm::vec3(0.0f, 1.0f, 0.0f));
            glm::quat quatZ = glm::angleAxis(m_euler.z, glm::vec3(0.0f, 0.0f, 1.0f));
            m_direction = quatZ * quatY * quatX * glm::vec3(0.0f, 0.0f, -1.0f);

            m_view = glm::lookAtRH(m_transform->GetPosition(), m_transform->GetPosition() + m_direction, glm::vec3(0.0f, 1.0f, 0.0f));
            m_projection = glm::perspective(glm::radians(m_fov), (float)m_app->GetWindowSize().x / (float)m_app->GetWindowSize().y, m_zNear, m_zFar);
            m_viewProjection = m_projection * m_view;

            if (updateMouseLook == K_TRUE)
            {
                double x = 0.0, y = 0.0;
                glfwGetCursorPos((GLFWwindow*)m_app->GetWindow(), &x, &y);
                static glm::vec2 cursorPosition = glm::vec2(0.0f);
                static glm::vec2 prevCursorPosition = cursorPosition;
                prevCursorPosition = cursorPosition;
                cursorPosition = glm::vec2(x, y);
                glm::vec2 mouseDelta = prevCursorPosition - cursorPosition;
                AddEuler(mCamera::eEulerAngle::PITCH, mouseDelta.y * 0.01f);
                AddEuler(mCamera::eEulerAngle::YAW, mouseDelta.x * 0.01f);
            }
            
            if (updateMovement == K_TRUE)
            {
                float forward = m_app->GetKey('W') * 0.1f;
                float backward = m_app->GetKey('S') * 0.1f;
                float left = m_app->GetKey('A') * 0.1f;
                float right = m_app->GetKey('D') * 0.1f;
                if (forward > 0.0f || backward > 0.0f || left > 0.0f || right > 0.0f)
                {
                    Move(forward);
                    Move(-backward);
                    Strafe(-left);
                    Strafe(right);

                    m_isMoving = K_TRUE;
                }
                else
                {
                    m_isMoving = K_FALSE;
                }
            }
        }

        void mCamera::AddEuler(const mCamera::eEulerAngle& angle, float value)
        {
            m_euler[(int)angle] += value;
        }

        void mCamera::Move(float value)
        {
            m_transform->GetPositionRef() += m_direction * value;

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

        void mCamera::Strafe(float value)
        {
            glm::vec3 right = glm::cross(m_direction, glm::vec3(0.0f, 1.0f, 0.0f));
            m_transform->GetPositionRef() += right * value;

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

        void mCamera::Lift(float value)
        {
            m_transform->GetPositionRef().y += value;

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