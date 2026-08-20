// game_object_pool.hpp

#pragma once

#include "object_pool.hpp"

namespace triton
{
    class XGameObjectPool : public XObjectPool<SGameObjectData>
    {
        TRITON_OBJECT(XGameObjectPool)

    public:
        using XObjectPool<SGameObjectData>::XObjectPool;
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