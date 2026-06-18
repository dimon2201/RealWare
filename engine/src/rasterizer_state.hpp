// rasterizer_state.hpp

#pragma once

#include "math.hpp"
#include "types.hpp"

namespace triton
{
    class SDepthState final
    {
    public:
        SDepthState(types::boolean useDepthTest_, types::boolean useDepthWrite_) : useDepthTest(useDepthTest_), useDepthWrite(useDepthWrite_) {}

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
        eBlendFactor srcFactors[8] = { eBlendFactor::ZERO };
        eBlendFactor dstFactors[8] = { eBlendFactor::ZERO };
    };

    class SViewport final
    {
    public:
        cVector4 rect = cVector4(0.0f);
    };
}