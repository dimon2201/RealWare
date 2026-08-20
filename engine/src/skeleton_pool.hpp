// skeleton_pool.hpp

#pragma once

#include "object_pool.hpp"
#include "skeleton.hpp"

namespace triton
{
    class CSkeletonPool : public CObjectPool<XSkeleton>
    {
        TRITON_CLASS_NAME(CSkeletonPool)

    public:
        using CObjectPool<XSkeleton>::CObjectPool;
        ~CSkeletonPool() override = default;

        XSkeleton::TGPULayout ConvertToGpuLayout(const XSkeleton& object) override
        {
            XSkeleton::TGPULayout gpul;

            return gpul;
        }

        void Update() override
        {
            Upload();
        }
    };
}