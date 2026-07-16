// render_instance.hpp

#pragma once

#include "math.hpp"
#include "handles.hpp"
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

    inline types::dword EvaluatePropertyBits(
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

    struct SRenderInstanceData
    {
        ERenderInstanceMotionType usage = ERenderInstanceMotionType::Static;
        HGameObject gameObject;
        HBatch batch;
        HMaterial material;
        HSkeleton skeleton;
        HRenderInstance instance;
        cMatrix4 worldMatrix = cMatrix4();
    };

    class SGPURenderInstanceLayout
    {
    public:
        SGPURenderInstanceLayout() = default;
        SGPURenderInstanceLayout(types::s32 materialIndex, const cTransform& transform);

        SGPURenderInstanceLayout(
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