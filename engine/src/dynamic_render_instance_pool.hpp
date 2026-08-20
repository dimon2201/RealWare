// dynamic_render_instance_pool.hpp

#pragma once

#include "object_pool.hpp"

namespace triton
{
    class XDynamicRenderInstancePool : public XObjectPool<SDynamicRenderInstanceData>
    {
        TRITON_OBJECT(XDynamicRenderInstancePool)

    public:
        using XObjectPool<SDynamicRenderInstanceData>::XObjectPool;
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