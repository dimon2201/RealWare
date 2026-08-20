// render_pass_pool.hpp

#pragma once

#include "object_pool.hpp"

namespace triton
{
    class CRenderPassPool : public CObjectPool<XRenderPass>
    {
        TRITON_CLASS_NAME(CRenderPassPool)

    public:
        using CObjectPool<XRenderPass>::CObjectPool;
        ~CRenderPassPool() override = default;

        XRenderPass::TGPULayout ConvertToGpuLayout(const XRenderPass& object) override
        {
            XRenderPass::TGPULayout gpul;

            return gpul;
        }

        void Update() override
        {
            Upload();
        }
    };
}