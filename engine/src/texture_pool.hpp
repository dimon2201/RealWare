// texture_pool.hpp

#pragma once

#include "object_pool_base.hpp"

namespace triton
{
    class XTexturePool : public XObjectPoolBase<STextureData>
    {
        TRITON_OBJECT(XTexturePool)

    public:
        using XObjectPoolBase<STextureData>::XObjectPoolBase;
        ~XTexturePool() override = default;

        STextureData::TGPULayout ConvertToGpuLayout(const STextureData& object) override
        {
            STextureData::TGPULayout gpul;

            return gpul;
        }

        void Update() override
        {
            Upload();
        }
    };
}