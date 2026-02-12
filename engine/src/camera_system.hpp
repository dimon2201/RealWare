// camera_system.hpp

#pragma once

#include "../../thirdparty/glm/glm/glm.hpp"
#include "category.hpp"
#include "component.hpp"
#include "system.hpp"
#include "math.hpp"
#include "types.hpp"

namespace triton
{
    class cCameraSystem : public ecs::cSystem
    {
        TRITON_OBJECT(cCameraSystem)

    public:
        explicit cCameraSystem(cContext* context);
        virtual ~cCameraSystem() override final = default;

        virtual void OnFrameUpdate() override final;

        void Update();
        void AddEuler(eCategory angle, types::f32 value);
        void Move(types::f32 value);
        void Strafe(types::f32 value);
        void Lift(types::f32 value);
    };
}