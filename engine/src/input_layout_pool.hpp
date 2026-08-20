// input_layout_pool.hpp

#pragma once

#include "object_pool.hpp"

namespace triton
{
    class CInputLayoutPool : public CObjectPool<XInputLayout>
    {
        TRITON_CLASS_NAME(CInputLayoutPool)

    public:
        using CObjectPool<XInputLayout>::CObjectPool;
        ~CInputLayoutPool() override = default;

        XInputLayout::TGPULayout ConvertToGpuLayout(const XInputLayout& object) override
        {
            XInputLayout::TGPULayout gpul;

            return gpul;
        }

        void Update() override
        {
            Upload();
        }
    };
}