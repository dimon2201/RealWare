// batch_pool.hpp

#pragma once

#include "object_pool.hpp"

namespace triton
{
    class XBatchPool : public XObjectPool<SBatchData>
    {
        TRITON_OBJECT(XBatchPool)

    public:
        using XObjectPool<SBatchData>::XObjectPool;
        ~XBatchPool() override = default;

        SBatchData::TGPULayout ConvertToGpuLayout(const SBatchData& object) override
        {
            SBatchData::TGPULayout gpul;

            return gpul;
        }

        void Update() override
        {
            Upload();
        }
    };
}