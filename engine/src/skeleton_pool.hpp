// skeleton_pool.hpp

#pragma once

#include "object_pool.hpp"

namespace triton
{
    class XSkeletonPool : public XObjectPool<SSkeletonData>
    {
        TRITON_OBJECT(XSkeletonPool)

    public:
        using XObjectPool<SSkeletonData>::XObjectPool;
        ~XSkeletonPool() override = default;

        SSkeletonData::TGPULayout ConvertToGpuLayout(const SSkeletonData& object) override
        {
            SSkeletonData::TGPULayout gpul;

            return gpul;
        }

        void Update() override
        {
            Upload();
        }
    };
}