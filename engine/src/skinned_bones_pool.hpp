// skinned_bones_pool.hpp

#pragma once

#include "object_pool.hpp"
#include "skinned_bone.hpp"

namespace triton
{
    class CSkinnedBonesPool : public CObjectPool<CSkinnedBone>
    {
        TRITON_CLASS_NAME(CSkinnedBonesPool)

    public:
        using CObjectPool<CSkinnedBone>::CObjectPool;
        ~CSkinnedBonesPool() override = default;

        CSkinnedBone::TGPULayout ConvertToGpuLayout(const CSkinnedBone& object) override
        {
            CSkinnedBone::TGPULayout gpul;
            gpul.modelMatrix = object.GetModelMatrix();

            return gpul;
        }

        void Update() override
        {
            Upload();
        }
    };
}