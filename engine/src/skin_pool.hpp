// skin_pool.hpp

#pragma once

#include "object_pool_base.hpp"

namespace triton
{
    class XSkinPool : public XObjectPoolBase<SSkinData>
    {
    public:
        using XObjectPoolBase<SSkinData>::XObjectPoolBase;
        ~XSkinPool() override = default;

        SSkinData::TGPULayout ConvertToGpuLayout(const SSkinData& object) override
        {
            SSkinData::TGPULayout gpul;

            return gpul;
        }

        void Update() override
        {
            Upload();
        }
    };
}