// rasterizer.hpp

#pragma once

#include "math.hpp"
#include "types.hpp"

namespace triton
{
    struct sDepthMode
    {
        types::boolean useDepthTest = types::K_TRUE;
        types::boolean useDepthWrite = types::K_TRUE;
    };

    struct sBlendMode
    {
        enum class eBlendFactor
        {
            ZERO = 0,
            ONE = 1,
            SRC_COLOR = 2,
            INV_SRC_COLOR = 3,
            SRC_ALPHA = 4,
            INV_SRC_ALPHA = 5
        };

        types::usize factorCount = 0;
        eBlendFactor srcFactors[8] = { eBlendFactor::ZERO };
        eBlendFactor dstFactors[8] = { eBlendFactor::ZERO };
    };

    struct sViewport
    {
        cVector4 rect = cVector4(0.0f);
    };
}