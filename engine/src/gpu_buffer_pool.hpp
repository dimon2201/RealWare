// gpu_buffer_pool.hpp

#pragma once

#include "object_pool.hpp"

namespace triton
{
    class XGPUBufferPool : public XObjectPool<XGPUBuffer>
    {
        TRITON_OBJECT(XGPUBufferPool)

    public:
        using XObjectPool<XGPUBuffer>::XObjectPool;
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