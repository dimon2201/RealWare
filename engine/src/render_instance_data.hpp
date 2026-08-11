// render_instance_data.hpp

#pragma once

#include "math.hpp"
#include "batch_data.hpp"
#include "material_data.hpp"
#include "skeleton_data.hpp"
#include "render_instance_motion_type.hpp"
#include "types.hpp"

namespace triton
{
    enum class ERenderInstancePropertyBit
    {
        Dynamic = 1,
        Skinned = 2
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

    struct SStaticRenderInstanceData
    {
        struct THandle : public SHandle {};

        struct TGPULayout
        {
            TGPULayout() = default;
            TGPULayout(types::s32 materialIndex, const cTransform& transform);
            TGPULayout(
                types::s32 materialIndex,
                types::s32 skeletonIndex,
                ERenderInstanceMotionType motionType,
                types::boolean isSkinned
            ) :
                _materialIndex(materialIndex),
                _skeletonIndex(skeletonIndex),
                _propertyBits(EvaluatePropertyBits(motionType, isSkinned)) {
            }

            types::f32 _use2D = 0.0f;
            types::s32 _materialIndex = -1;
            types::s32 _skeletonIndex = -1;
            types::dword _propertyBits = 0;
            cMatrix4 _world = cMatrix4(1.0f);
        };

        ERenderInstanceMotionType usage = ERenderInstanceMotionType::Static;
        SBatchData::THandle batch;
        SMaterialData::THandle material;
        SSkeletonData::THandle skeleton;
        SStaticRenderInstanceData::THandle instance;
        cMatrix4 worldMatrix = cMatrix4();
    };

    struct SDynamicRenderInstanceData
    {
        struct THandle : public SHandle {};

        struct TGPULayout
        {
            TGPULayout() = default;
            TGPULayout(types::s32 materialIndex, const cTransform& transform);
            TGPULayout(
                types::s32 materialIndex,
                types::s32 skeletonIndex,
                ERenderInstanceMotionType motionType,
                types::boolean isSkinned
            ) :
                _materialIndex(materialIndex),
                _skeletonIndex(skeletonIndex),
                _propertyBits(EvaluatePropertyBits(motionType, isSkinned)) {
            }

            types::f32 _use2D = 0.0f;
            types::s32 _materialIndex = -1;
            types::s32 _skeletonIndex = -1;
            types::dword _propertyBits = 0;
            cMatrix4 _world = cMatrix4(1.0f);
        };

        ERenderInstanceMotionType usage = ERenderInstanceMotionType::Static;
        SBatchData::THandle batch;
        SMaterialData::THandle material;
        SSkeletonData::THandle skeleton;
        SDynamicRenderInstanceData::THandle instance;
        cMatrix4 worldMatrix = cMatrix4();
    };
}