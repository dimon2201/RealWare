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
            SDynamicRenderInstanceData::TGPULayout gpul;
            gpul._use2D = 0.0f;
            gpul._world = object.worldMatrix;
            gpul._skinnedBoneBufferOffset = object.skinnedBoneBufferOffset;
            gpul._materialIndex = _context->GetSubsystem<XMaterialSubsystem>()->GetPool()->GetPackedIndex(object.material);

            return gpul;
        }

        void Update() override
        {
            Upload();
        }
    };
}