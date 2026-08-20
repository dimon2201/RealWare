// render_pass_pool.hpp

#pragma once

#include "object_pool.hpp"

namespace triton
{
    class XRenderPassPool : public XObjectPool<XRenderPass>
    {
        TRITON_OBJECT(XRenderPassPool)

    public:
        using XObjectPool<XRenderPass>::XObjectPool;
        ~XRenderPassPool() override = default;

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