// skinned_bones_pool.hpp

#pragma once

#include "object_pool_base.hpp"

namespace triton
{
    class XSkinnedBonesPool : public XObjectPoolBase<SSkinnedBoneData>
    {
    public:
        using XObjectPoolBase<SSkinnedBoneData>::XObjectPoolBase;
        ~XSkinnedBonesPool() override = default;

        SSkinnedBoneData::TGPULayout ConvertToGpuLayout(const SSkinnedBoneData& object) override
        {
            SSkinnedBoneData::TGPULayout gpul;
            gpul.modelMatrix = object.modelMatrix;

            return gpul;
        }

        void Update() override
        {
            Upload();
        }
    };
}