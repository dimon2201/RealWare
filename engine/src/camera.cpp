// camera.cpp

#include "camera.hpp"
#include "context.hpp"
#include "time.hpp"
#include "render_pass.hpp"
#include "thread_guard.hpp"
#include "input_backend.hpp"
#include "shader_pool.hpp"
#include "render_pass_geometry.hpp"

using namespace types;

void triton::XCamera::Bind(XRenderPassGeometry* pass)
{
    CThreadGuard::AssertRender();

    XShader::THandle shaderHandle = pass->GetShader();
    XShader& shader = *_context->GetPool<CShaderPool>()->Get(shaderHandle);

    // TODO: [Vulkan backend] This must be done using render command queue on main thread
    // ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
    // VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
    //IGraphicsBackend* gfxBackend = _context->GetBackend<IGraphicsBackend>();
    //gfxBackend->SetShaderUniform(shader.GetGPUResource(), "viewProjectionMatrix", _viewProjectionMatrix.Get());
}

void triton::XCamera::Update(const cVector2& mouseDelta, usize screenWidth, usize screenHeight, f32 fov, f32 zNear, f32 zFar, f32 mouseSensitivity)
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
    AddEuler(cMath::EEulerAngle::PITCH, -mouseDelta.GetY() * mouseSensitivity * deltaTime);
    AddEuler(cMath::EEulerAngle::YAW, -mouseDelta.GetX() * mouseSensitivity * deltaTime);
    
    const f32 camSpeed = 1.0f;
    IInputBackend* ib = _context->GetBackend<IInputBackend>();
    EKeyState ksW = ib->GetKey(EKeyCode::W);
    EKeyState ksS = ib->GetKey(EKeyCode::S);
    EKeyState ksA = ib->GetKey(EKeyCode::A);
    EKeyState ksD = ib->GetKey(EKeyCode::D);
    const f32 forward = ksW == EKeyState::Pressed ? camSpeed * deltaTime : 0.0f;
    const f32 backward = ksS == EKeyState::Pressed ? camSpeed * deltaTime : 0.0f;
    const f32 left = ksA == EKeyState::Pressed ? camSpeed * deltaTime : 0.0f;
    const f32 right = ksD == EKeyState::Pressed ? camSpeed * deltaTime : 0.0f;
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