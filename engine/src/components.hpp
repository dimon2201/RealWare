#pragma once

#include "math.hpp"
#include "stack_value.hpp"
#include "render_instance_data.hpp"
#include "geometry_view.hpp"

namespace triton
{
    class cAudioBackendSound;
}

namespace triton::ecs::components
{
    class SComponent {};

    class STransformComponent final : public SComponent
    {
    public:
        cMatrix4 _world = cMatrix4(1.0f);
    };

    class SRenderInstanceComponent final : public SComponent
    {
    public:
        ERenderInstanceMotionType _usage = ERenderInstanceMotionType::Static;
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

    struct SGeometryComponent : public SComponent
    {
        SGeometryView geometry = {};
    };

    class sSoundComponent : public SComponent
    {
    public:
        cAudioBackendSound* backendSound = nullptr;
    };
}