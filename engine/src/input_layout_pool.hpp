// input_layout_pool.hpp

#pragma once

#include "object_pool.hpp"

namespace triton
{
    class XInputLayoutPool : public XObjectPool<XInputLayout>
    {
        TRITON_OBJECT(XInputLayoutPool)

    public:
        using XObjectPool<XInputLayout>::XObjectPool;
        ~XInputLayoutPool() override = default;

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