// model3d_pool.hpp

#pragma once

#include "object_pool_base.hpp"

namespace triton
{
    class XModel3DPool : public XObjectPoolBase<SModel3DData>
    {
    public:
        using XObjectPoolBase<SModel3DData>::XObjectPoolBase;
        ~XModel3DPool() override = default;

        SModel3DData::TGPULayout ConvertToGpuLayout(const SModel3DData& object) override
        {
            SModel3DData::TGPULayout gpul;

            return gpul;
        }

        void Update() override
        {
            Upload();
        }
    };
}