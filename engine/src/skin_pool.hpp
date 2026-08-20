// skin_pool.hpp

#pragma once

#include "object_pool.hpp"

namespace triton
{
    class XSkinPool : public XObjectPool<SSkinData>
    {
        TRITON_OBJECT(XSkinPool)

    public:
        using XObjectPool<SSkinData>::XObjectPool;
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