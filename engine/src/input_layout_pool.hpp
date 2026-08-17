// input_layout_pool.hpp

#pragma once

#include "object_pool_base.hpp"

namespace triton
{
    class XInputLayoutPool : public XObjectPoolBase<XInputLayout>
    {
    public:
        using XObjectPoolBase<XInputLayout>::XObjectPoolBase;
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