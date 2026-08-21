// rasterizer_state.hpp

#pragma once

#include "math.hpp"
#include "types.hpp"

namespace triton
{
    struct SClearState final
    {
        SClearState() = default;
        SClearState(const cVector4& color, types::f32 depth) : color(color), depth(depth) {}

        cVector4 color = cVector4(1.0f);
        types::f32 depth = 1.0f;
    };

    class SDepthState final
    {
    public:
        SDepthState() = default;
        SDepthState(types::boolean useDepthTest_, types::boolean useDepthWrite_)
            : useDepthTest(useDepthTest_), useDepthWrite(useDepthWrite_) {}

        types::boolean useDepthTest = types::K_TRUE;
        types::boolean useDepthWrite = types::K_TRUE;
    };

    enum class EBlendFactor
    {
        ZERO,
        ONE,
        SRC_COLOR,
        INV_SRC_COLOR,
        SRC_ALPHA,
        INV_SRC_ALPHA
    };

    class SBlendState final
    {
    public:
        types::usize factorCount = 0;
        EBlendFactor srcFactors[8] = { EBlendFactor::ZERO };
        EBlendFactor dstFactors[8] = { EBlendFactor::ZERO };
    };

    class SViewport final
    {
    public:
        cVector4 rect = cVector4(0.0f);
    };
}