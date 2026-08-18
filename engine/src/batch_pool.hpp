// batch_pool.hpp

#pragma once

#include "object_pool_base.hpp"

namespace triton
{
    class XBatchPool : public XObjectPoolBase<SBatchData>
    {
        TRITON_OBJECT(XBatchPool)

    public:
        using XObjectPoolBase<SBatchData>::XObjectPoolBase;
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