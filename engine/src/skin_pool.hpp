// skin_pool.hpp

#pragma once

#include "object_pool.hpp"

namespace triton
{
    class CSkinPool : public CObjectPool<XSkin>
    {
        TRITON_CLASS_NAME(CSkinPool)

    public:
        using CObjectPool<XSkin>::CObjectPool;
        ~CSkinPool() override = default;

        XSkin::TGPULayout ConvertToGpuLayout(const XSkin& object) override
        {
            XSkin::TGPULayout gpul;

            return gpul;
        }

        void Update() override
        {
            Upload();
        }
    };
}