// render_instance_pool.hpp

#pragma once

#include "object_pool.hpp"
#include "skeleton_pool.hpp"
#include "material_pool.hpp"
#include "math.hpp"
#include "world_render_instance.hpp"

namespace triton
{
    class CRenderInstancePool : public CObjectPool<XRenderInstance>
    {
        TRITON_CLASS_NAME(CRenderInstancePool)

    public:
        using CObjectPool<XRenderInstance>::CObjectPool;
        ~CRenderInstancePool() override = default;

        XRenderInstance::TGPULayout ConvertToGpuLayout(const XRenderInstance& object) override
        {
            XRenderInstance::TGPULayout gpul;
            gpul._use2D = 0.0f;
            gpul._world = object.GetWorldMatrix();
            gpul._skinnedBoneBufferOffset = object.GetSkinnedBoneBufferOffset();
            gpul._materialIndex = _context->GetPool<CMaterialPool>()->GetPackedIndex(object.GetMaterial());

            return gpul;
        }

        void Update() override
        {
            Upload();
        }
    };
}