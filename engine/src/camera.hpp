// camera.hpp

#pragma once

#include "math.hpp"
#include "object.hpp"
#include "types.hpp"

namespace triton
{
    class XRenderPass;

    class XCamera : public iObject
    {
        TRITON_OBJECT(XCamera)

        cVector3 _eulerAngles = cVector3(0.0f);
        cVector3 _worldPosition = cVector3(0.0f);
        cVector3 _worldDirection = cVector3(0.0f);
        cMatrix4 _viewMatrix = cMatrix4(1.0f);
        cMatrix4 _projectionMatrix = cMatrix4(1.0f);
        cMatrix4 _viewProjectionMatrix = cMatrix4(1.0f);
        cVector2 _previousCursorPosition = cVector2(0.0f);
        cVector2 _cursorPosition = cVector2(0.0f);

    public:
        explicit XCamera(cContext* context) : iObject(context) {}
        ~XCamera() override = default;

        void Bind(XRenderPass* pass);
        void Update(const cVector2& screenCursorPosition, types::usize screenWidth, types::usize screenHeight, types::f32 fov, types::f32 zNear, types::f32 zFar, types::f32 mouseSensitivity);
        void AddEuler(cMath::EEulerAngle angle, types::f32 value);
        void Move(types::f32 value);
        void Strafe(types::f32 value);
        void Lift(types::f32 value);
    };
}