// render_instance.hpp

#pragma once

#include "math.hpp"
#include "types.hpp"

namespace triton
{
    enum class ERenderInstancePropertyBit
    {
        Dynamic = 1,
        Skinned = 2
    };

    enum class ERenderInstanceMotionType
    {
        Static,
        Dynamic
    };

    types::dword EvaluatePropertyBits(
        ERenderInstanceMotionType motionType,
        types::boolean isSkinned
    )
    {
        types::dword pb = 0;
        if (motionType == ERenderInstanceMotionType::Dynamic)
            pb |= (types::dword)ERenderInstancePropertyBit::Dynamic;
        if (isSkinned == types::K_TRUE)
            pb |= (types::dword)ERenderInstancePropertyBit::Skinned;

        return pb;
    }

    class SRenderInstance final
    {
    public:
        SRenderInstance() = default;
        SRenderInstance(types::s32 materialIndex, const cTransform& transform);

        SRenderInstance(
            types::s32 materialIndex,
            types::s32 skeletonIndex,
            ERenderInstanceMotionType motionType,
            types::boolean isSkinned
        )
            :
            _materialIndex(materialIndex),
            _skeletonIndex(skeletonIndex),
            _propertyBits(EvaluatePropertyBits(motionType, isSkinned)) {}

        types::f32 _use2D = 0.0f;
        types::s32 _materialIndex = -1;
        types::s32 _skeletonIndex = -1;
        types::dword _propertyBits = 0;
        cMatrix4 _world = cMatrix4(1.0f);
    };
}