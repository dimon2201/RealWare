// skinned_bones_pool.hpp

#pragma once

#include "object_pool.hpp"

namespace triton
{
    class XSkinnedBonesPool : public XObjectPool<SSkinnedBoneData>
    {
        TRITON_OBJECT(XSkinnedBonesPool)

    public:
        using XObjectPool<SSkinnedBoneData>::XObjectPool;
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