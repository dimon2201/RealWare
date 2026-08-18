// render_pass_pool.hpp

#pragma once

#include "object_pool_base.hpp"

namespace triton
{
    class XRenderPassPool : public XObjectPoolBase<XRenderPass>
    {
        TRITON_OBJECT(XRenderPassPool)

    public:
        using XObjectPoolBase<XRenderPass>::XObjectPoolBase;
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