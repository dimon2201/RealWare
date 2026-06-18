// rasterizer_state.hpp

#pragma once

#include "math.hpp"
#include "types.hpp"

namespace triton
{
    class SDepthState final
    {
    public:
        types::boolean useDepthTest = types::K_TRUE;
        types::boolean useDepthWrite = types::K_TRUE;
    };

    enum class EBlendFactor
    {
        ZER,
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
        eBlendFactor srcFactors[8] = { eBlendFactor::ZERO };
        eBlendFactor dstFactors[8] = { eBlendFactor::ZERO };
    };

    class SViewport final
    {
    public:
        cVector4 rect = cVector4(0.0f);
    };
}