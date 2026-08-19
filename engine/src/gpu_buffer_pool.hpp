// gpu_buffer_pool.hpp

#pragma once

#include "object_pool_base.hpp"

namespace triton
{
    class XGPUBufferPool : public XObjectPoolBase<XGPUBuffer>
    {
        TRITON_OBJECT(XGPUBufferPool)

    public:
        using XObjectPoolBase<XGPUBuffer>::XObjectPoolBase;
        ~XGPUBufferPool() override = default;

        XGPUBuffer::TGPULayout ConvertToGpuLayout(const XGPUBuffer& object) override
        {
            XGPUBuffer::TGPULayout gpul;

            return gpul;
        }

        void Update() override
        {
            Upload();
        }
    };
}