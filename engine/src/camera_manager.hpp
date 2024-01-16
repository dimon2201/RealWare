#pragma once

#include "../../thirdparty/glm/glm/glm.hpp"
#include "ecs.hpp"
#include "types.hpp"

namespace realware
{
    namespace render
    {
        class cRenderContext;
        struct sRenderPass;
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

            mCamera() {}
            ~mCamera() {}

            void Init();
            void Free();
            void Update(entity object, cScene* scene, boolean updateMouseLook, boolean updateMovement);
            void AddEuler(sCCamera* camera, const eEulerAngle& angle, float value);
            void Move(cScene* scene, sCCamera* camera, sCTransform* transform, float value);
            void Strafe(cScene* scene, sCCamera* camera, sCTransform* transform, float value);
            void Lift(cScene* scene, sCCamera* camera, sCTransform* transform, float value);

        private:
            render::cRenderContext* m_context;

        };
    }
}