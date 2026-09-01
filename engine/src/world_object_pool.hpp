// world_object_pool.hpp

#pragma once

#include "object_pool.hpp"
#include "world_object.hpp"

namespace triton
{
    class CWorldObjectPool : public CObjectPool<XWorldObject>
    {
        TRITON_CLASS_NAME(CWorldObjectPool)

    public:
        using CObjectPool<XWorldObject>::CObjectPool;
        ~CWorldObjectPool() override = default;

        XWorldObject::TGPULayout ConvertToGpuLayout(const XWorldObject& object) override
        {
            XWorldObject::TGPULayout gpul;

            return gpul;
        }

        void Update() override
        {
            Upload();
        }
    };
}