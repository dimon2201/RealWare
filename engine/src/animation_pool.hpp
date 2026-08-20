// animation_pool.hpp

#pragma once

#include "object_pool.hpp"

namespace triton
{
    class XAnimationPool : public XObjectPool<SAnimationData>
    {
        TRITON_OBJECT(XAnimationPool)

    public:
        using XObjectPool<SAnimationData>::XObjectPool;
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