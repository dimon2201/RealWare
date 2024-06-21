#pragma once

#include "../../thirdparty/glm/glm/glm.hpp"
#include "ecs.hpp"
#include "render_manager.hpp"
#include "types.hpp"

namespace realware
{
    namespace render
    {
        class cRenderContext;
        struct sRenderPass;
        class cTransform;
    }

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

            mCamera(cApplication* app);
            ~mCamera();

            void Update(boolean updateMouseLook, boolean updateMovement);
            void AddEuler(const eEulerAngle& angle, float value);
            void Move(float value);
            void Strafe(float value);
            void Lift(float value);

            inline glm::mat4 GetViewProjectionMatrix() { return m_viewProjection; }

        private:
            cApplication* m_app;
            render::cRenderContext* m_context;
            render::sTransform m_transform;
            glm::vec3 m_euler = glm::vec3(0.0f);
            glm::vec3 m_direction = glm::vec3(0.0f);
            glm::mat4 m_view = glm::mat4(1.0f);
            glm::mat4 m_projection = glm::mat4(1.0f);
            glm::mat4 m_viewProjection = glm::mat4(1.0f);
            core::f32 m_fov = 60.0f;
            core::f32 m_zNear = 0.01f;
            core::f32 m_zFar = 100.0f;
            core::boolean m_isMoving = K_FALSE;

        };
    }
}