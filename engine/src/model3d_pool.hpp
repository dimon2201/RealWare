// model3d_pool.hpp

#pragma once

#include "object_pool.hpp"

namespace triton
{
    class XModel3DPool : public XObjectPool<SModel3DData>
    {
        TRITON_OBJECT(XModel3DPool)

    public:
        using XObjectPool<SModel3DData>::XObjectPool;
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