// skeleton_pool.hpp

#pragma once

#include "object_pool_base.hpp"

namespace triton
{
    class XSkeletonPool : public XObjectPoolBase<SSkeletonData>
    {
    public:
        using XObjectPoolBase<SSkeletonData>::XObjectPoolBase;
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