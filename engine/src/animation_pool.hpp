// animation_pool.hpp

#pragma once

#include "object_pool_base.hpp"

namespace triton
{
    class XAnimationPool : public XObjectPoolBase<SAnimationData>
    {
        TRITON_OBJECT(XAnimationPool)

    public:
        using XObjectPoolBase<SAnimationData>::XObjectPoolBase;
        ~XAnimationPool() override = default;

        SAnimationData::TGPULayout ConvertToGpuLayout(const SAnimationData& object) override
        {
            SAnimationData::TGPULayout gpul;

            return gpul;
        }

        void Update() override
        {
            Upload();
        }
    };
}