// render_target_pool.hpp

#pragma once

#include "object_pool.hpp"

namespace triton
{
    class XRenderTargetPool : public XObjectPool<XRenderTarget>
    {
        TRITON_OBJECT(XRenderTargetPool)

    public:
        using XObjectPool<XRenderTarget>::XObjectPool;
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