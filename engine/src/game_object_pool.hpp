// game_object_pool.hpp

#pragma once

#include "object_pool_base.hpp"

namespace triton
{
    class XGameObjectPool : public XObjectPoolBase<SGameObjectData>
    {
        TRITON_OBJECT(XGameObjectPool)

    public:
        using XObjectPoolBase<SGameObjectData>::XObjectPoolBase;
        ~XGameObjectPool() override = default;

        SGameObjectData::TGPULayout ConvertToGpuLayout(const SGameObjectData& object) override
        {
            SGameObjectData::TGPULayout gpul;

            return gpul;
        }

        void Update() override
        {
            Upload();
        }
    };
}