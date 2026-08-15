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
            SStaticRenderInstanceData::TGPULayout gpul;
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