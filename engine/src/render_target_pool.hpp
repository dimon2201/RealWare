// render_target_pool.hpp

#pragma once

#include "object_pool_base.hpp"

namespace triton
{
    class XRenderTargetPool : public XObjectPoolBase<XRenderTarget>
    {
    public:
        using XObjectPoolBase<XRenderTarget>::XObjectPoolBase;
        ~XRenderTargetPool() override = default;

        XRenderTarget::TGPULayout ConvertToGpuLayout(const XRenderTarget& object) override
        {
            XRenderTarget::TGPULayout gpul;

            return gpul;
        }

        void Update() override
        {
            Upload();
        }
    };
}