// animation_pool.hpp

#pragma once

#include "object_pool.hpp"

namespace triton
{
    class CAnimationPool : public CObjectPool<XAnimation>
    {
        TRITON_CLASS_NAME(CAnimationPool)

    public:
        using CObjectPool<XAnimation>::CObjectPool;
        ~CAnimationPool() override = default;

        XAnimation::TGPULayout ConvertToGpuLayout(const XAnimation& object) override
        {
            XAnimation::TGPULayout gpul;

            return gpul;
        }

        void Update() override
        {
            Upload();
        }
    };
}