#pragma once

#include "math.hpp"
#include "stack_value.hpp"
#include "render_instance.hpp"

namespace triton
{
    class cAudioBackendSound;
}

namespace triton::ecs::components
{
    class SComponent : public cStackValue {};

    class STransformComponent final : public SComponent
    {
    public:
        cMatrix4 _world = cMatrix4(1.0f);
    };

    class SRenderInstanceComponent final : public SComponent
    {
    public:
        SRenderInstance::EUsage _usage = SRenderInstance::EUsage::NONE;
    };

    class SCameraComponent : public SComponent
    {
    public:
        ::triton::cMatrix4 _view = ::triton::cMatrix4(1.0f);
        ::triton::cMatrix4 _projection = ::triton::cMatrix4(1.0f);
        ::triton::cMatrix4 _viewProjection = ::triton::cMatrix4(1.0f);
        types::f32 _fov = 60.0f;
        types::f32 _zNear = 0.01f;
        types::f32 _zFar = 100.0f;
    };

    class sSoundComponent : public SComponent
    {
    public:
        cAudioBackendSound* backendSound = nullptr;
    };
}