#include "../../thirdparty/glm/glm/gtc/matrix_transform.hpp"
#include "../../thirdparty/glm/glm/gtc/quaternion.hpp"
#include "../../thirdparty/glm/glm/gtx/quaternion.hpp"
#include "camera_manager.hpp"
#include "user_input_manager.hpp"
#include "physics_manager.hpp"
#include "render_context.hpp"

extern realware::core::mUserInput* userInputManager;
extern realware::physics::mPhysics* physicsManager;

namespace realware
{
    namespace core
    {
        void mCamera::Init()
        {
        }

        void mCamera::Free()
        {
        }

        void mCamera::Update(entity object, cScene* scene, boolean updateMouseLook, boolean updateMovement)
        {
            sCTransform* transform = scene->Get<sCTransform>(object);
            sCCamera* camera = scene->Get<sCCamera>(object);

            if (camera->Euler.x > glm::radians(65.0f)) {
                camera->Euler.x = glm::radians(65.0f);
            }
            else if (camera->Euler.x < glm::radians(-65.0f)) {
                camera->Euler.x = glm::radians(-65.0f);
            }

            glm::quat quatX = glm::angleAxis(camera->Euler.x, glm::vec3(1.0f, 0.0f, 0.0f));
            glm::quat quatY = glm::angleAxis(camera->Euler.y, glm::vec3(0.0f, 1.0f, 0.0f));
            glm::quat quatZ = glm::angleAxis(camera->Euler.z, glm::vec3(0.0f, 0.0f, 1.0f));
            camera->Direction = quatZ * quatY * quatX * glm::vec3(0.0f, 0.0f, -1.0f);

            camera->View = glm::lookAtRH(transform->Position, transform->Position + camera->Direction, glm::vec3(0.0f, 1.0f, 0.0f));
            camera->Projection = glm::perspective(glm::radians(camera->FOV), (float)userInputManager->GetWindowSize().x / (float)userInputManager->GetWindowSize().y, camera->ZNear, camera->ZFar);
            camera->ViewProjection = camera->Projection * camera->View;

            if (updateMouseLook == K_TRUE)
            {
                glm::vec2 mouseDelta = userInputManager->GetCursorDelta();
                AddEuler(camera, mCamera::eEulerAngle::PITCH, mouseDelta.y * 0.01f);
                AddEuler(camera, mCamera::eEulerAngle::YAW, mouseDelta.x * 0.01f);
            }
            
            if (updateMovement == K_TRUE)
            {
                float forward = userInputManager->GetKey('W') * 0.1f;
                float backward = userInputManager->GetKey('S') * 0.1f;
                float left = userInputManager->GetKey('A') * 0.1f;
                float right = userInputManager->GetKey('D') * 0.1f;
                if (forward > 0.0f || backward > 0.0f || left > 0.0f || right > 0.0f)
                {
                    Move(scene, camera, transform, forward);
                    Move(scene, camera, transform, -backward);
                    Strafe(scene, camera, transform, -left);
                    Strafe(scene, camera, transform, right);

                    camera->IsMoving = K_TRUE;
                }
                else
                {
                    camera->IsMoving = K_FALSE;
                }
            }
        }

        void mCamera::AddEuler(sCCamera* camera, const mCamera::eEulerAngle& angle, float value)
        {
            camera->Euler[(int)angle] += value;
        }

        void mCamera::Move(cScene* scene, sCCamera* camera, sCTransform* transform, float value)
        {
            transform->Position += camera->Direction * value;

            sCPhysicsCharacterController* controller =
                scene->Get<sCPhysicsCharacterController>(camera->Owner);
            if (controller != nullptr)
            {
                physicsManager->SetCharacterControllerMovement(
                    { camera->Owner, scene },
                    camera->Direction * value
                );
            }
        }

        void mCamera::Strafe(cScene* scene, sCCamera* camera, sCTransform* transform, float value)
        {
            glm::vec3 right = glm::cross(camera->Direction, glm::vec3(0.0f, 1.0f, 0.0f));
            transform->Position += right * value;

            sCPhysicsCharacterController* controller =
                scene->Get<sCPhysicsCharacterController>(camera->Owner);
            if (controller != nullptr)
            {
                physicsManager->SetCharacterControllerMovement(
                    { camera->Owner, scene },
                    right * value
                );
            }
        }

        void mCamera::Lift(cScene* scene, sCCamera* camera, sCTransform* transform, float value)
        {
            transform->Position.y += value;

            sCPhysicsCharacterController* controller =
                scene->Get<sCPhysicsCharacterController>(camera->Owner);
            if (controller != nullptr)
            {
                physicsManager->SetCharacterControllerMovement(
                    { camera->Owner, scene },
                    glm::vec3(0.0f, value, 0.0f)
                );
            }
        }
    }
}