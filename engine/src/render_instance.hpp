// render_instance.hpp

#pragma once

#include "math.hpp"
#include "types.hpp"

namespace triton
{
    class SRenderInstance final
    {
    public:
        enum class EUsage : types::u32
        {
            NONE = 0,
            STATIC,
            DYNAMIC
        };

        SRenderInstance() = default;
        SRenderInstance(types::s32 materialIndex, const cTransform& transform);

        types::f32 _use2D = 0.0f;
        types::s32 _materialIndex = -1;
        types::dword _pad[2] = {};
        cMatrix4 _world = cMatrix4(1.0f);
    };
}