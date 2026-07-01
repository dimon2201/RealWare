// camera.cpp

#include "camera.hpp"
#include "context.hpp"
#include "time.hpp"
#include "graphics_pipeline_backend.hpp"
#include "render_pass.hpp"
#include "thread_guard.hpp"
#include "input.hpp"

using namespace types;

void triton::XCamera::Bind(XRenderPass* pass)
{
    CThreadGuard::AssertRender();

    CGPUShader shader = pass->GetShader()->GetGPUShader();

    iGraphicsPipelineBackend* gfxPipelineBackend = _context->GetBackend<iGraphicsPipelineBackend>();
    gfxPipelineBackend->SetShaderUniform(&shader, "ViewProjection", _viewProjectionMatrix.Get());
}

void triton::XCamera::Update(const cVector2& screenCursorPosition, usize screenWidth, usize screenHeight, f32 fov, f32 zNear, f32 zFar, f32 mouseSensitivity)
{
    const cTime* time = _context->GetSubsystem<cTime>();
    const cMath* math = _context->GetSubsystem<cMath>();

    const f32 deltaTime = 1.0f; //time->GetDeltaTime();
    if (_eulerAngles.GetX() > math->DegreesToRadians(65.0f))
        _eulerAngles.SetX(math->DegreesToRadians(65.0f));
    else if (_eulerAngles.GetX() < math->DegreesToRadians(-65.0f))
        _eulerAngles.SetX(math->DegreesToRadians(-65.0f));
    const cQuaternion quatX = cQuaternion(_eulerAngles.GetX(), cVector3(1.0f, 0.0f, 0.0f));
    const cQuaternion quatY = cQuaternion(_eulerAngles.GetY(), cVector3(0.0f, 1.0f, 0.0f));
    const cQuaternion quatZ = cQuaternion(_eulerAngles.GetZ(), cVector3(0.0f, 0.0f, 1.0f));
    _worldDirection = quatZ * quatY * quatX * cVector3(0.0f, 0.0f, -1.0f);

    _viewMatrix = cMatrix4(_worldPosition, _worldDirection, cVector3(0.0f, 1.0f, 0.0f));
    _projectionMatrix = cMatrix4(fov, (f32)screenWidth / screenHeight, zNear, zFar);
    _viewProjectionMatrix = _projectionMatrix * _viewMatrix;
    _previousCursorPosition = _cursorPosition;
    _cursorPosition = screenCursorPosition;
    const cVector2 mouseDelta = _previousCursorPosition - _cursorPosition;
    AddEuler(cMath::EEulerAngle::PITCH, mouseDelta.GetY() * mouseSensitivity * deltaTime);
    AddEuler(cMath::EEulerAngle::YAW, mouseDelta.GetX() * mouseSensitivity * deltaTime);
    
    cInput* input = _context->GetSubsystem<cInput>();
    const f32 forward = input->GetKeyPressed('W') * 0.01f * deltaTime;
    const f32 backward = input->GetKeyPressed('S') * 0.01f * deltaTime;
    const f32 left = input->GetKeyPressed('A') * 0.01f * deltaTime;
    const f32 right = input->GetKeyPressed('D') * 0.01f * deltaTime;
    if (forward > 0.0f || backward > 0.0f || left > 0.0f || right > 0.0f)
    {
        Move(forward);
        Move(-backward);
        Strafe(-left);
        Strafe(right);
    }
}

void triton::XCamera::AddEuler(cMath::EEulerAngle angle, f32 value)
{
    value = glm::radians(value);
    if (angle == cMath::EEulerAngle::PITCH)
        _eulerAngles.AddX(value);
    else if (angle == cMath::EEulerAngle::YAW)
        _eulerAngles.AddY(value);
    else if (angle == cMath::EEulerAngle::ROLL)
        _eulerAngles.AddZ(value);
}

void triton::XCamera::Move(f32 value)
{
    _worldPosition = _worldPosition + _worldDirection * value;
}

void triton::XCamera::Strafe(f32 value)
{
    const cVector3 up = cVector3(0.0f, 1.0f, 0.0f);
    const cVector3 right = _worldDirection.Cross(up);
    _worldPosition = _worldPosition + right * value;
}

void triton::XCamera::Lift(f32 value)
{
    const cVector3 up = cVector3(0.0f, 1.0f, 0.0f);
    _worldPosition = _worldPosition + up * value;
}