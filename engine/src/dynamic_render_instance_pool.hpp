// dynamic_render_instance_pool.hpp

#pragma once

#include "object_pool_base.hpp"

namespace triton
{
    class XDynamicRenderInstancePool : public XObjectPoolBase<SDynamicRenderInstanceData>
    {
    public:
        using XObjectPoolBase<SDynamicRenderInstanceData>::XObjectPoolBase;
        ~XDynamicRenderInstancePool() override = default;

        SDynamicRenderInstanceData::TGPULayout ConvertToGpuLayout(const SDynamicRenderInstanceData& object) override
        {
            auto skeletonIdxResult = _context->GetSubsystem<XSkeletonSubsystem>()->GetPool()->GetBufferIndex(object.skeleton);
            auto materialIdxResult = _context->GetSubsystem<XMaterialSubsystem>()->GetPool()->GetBufferIndex(object.material);

            SDynamicRenderInstanceData::TGPULayout gpul;
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