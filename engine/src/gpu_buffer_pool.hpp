// gpu_buffer_pool.hpp

#pragma once

#include "object_pool.hpp"

namespace triton
{
    class CGPUBufferPool : public CObjectPool<XGPUBuffer>
    {
        TRITON_CLASS_NAME(CGPUBufferPool)

    public:
        using CObjectPool<XGPUBuffer>::CObjectPool;
        ~CGPUBufferPool() override = default;

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