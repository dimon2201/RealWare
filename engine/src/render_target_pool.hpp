// render_target_pool.hpp

#pragma once

#include "object_pool.hpp"

namespace triton
{
    class CRenderTargetPool : public CObjectPool<XRenderTarget>
    {
        TRITON_CLASS_NAME(CRenderTargetPool)

    public:
        using CObjectPool<XRenderTarget>::CObjectPool;
        ~CRenderTargetPool() override = default;

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