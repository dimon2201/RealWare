// static_render_instance_pool.hpp

#pragma once

#include "object_pool_base.hpp"
#include "skeleton_pool.hpp"
#include "material_pool.hpp"

namespace triton
{
    class XStaticRenderInstancePool : public XObjectPoolBase<SStaticRenderInstanceData>
    {
    public:
        using XObjectPoolBase<SStaticRenderInstanceData>::XObjectPoolBase;
        ~XStaticRenderInstancePool() override = default;

        SStaticRenderInstanceData::TGPULayout ConvertToGpuLayout(const SStaticRenderInstanceData& object) override
        {
            auto skeletonIdxResult = _context->GetSubsystem<XSkeletonSubsystem>()->GetPool()->GetBufferIndex(object.skeleton);
            auto materialIdxResult = _context->GetSubsystem<XMaterialSubsystem>()->GetPool()->GetBufferIndex(object.material);

            SStaticRenderInstanceData::TGPULayout gpul;
            gpul._use2D = 0.0f;
            gpul._world = object.worldMatrix;
            gpul._skeletonIndex = skeletonIdxResult.has_value() ? skeletonIdxResult.value() : -1;
            gpul._materialIndex = materialIdxResult.has_value() ? materialIdxResult.value() : -1;

            return gpul;
        }

        void Update() override
        {
            Upload();
        }
    };
}