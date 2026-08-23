// texture_pool.hpp

#pragma once

#include "object_pool.hpp"
#include "texture.hpp"

namespace triton
{
    class PTexturePool : public CObjectPool<XTexture>
    {
        TRITON_CLASS_NAME(PTexturePool)

    public:
        using CObjectPool<XTexture>::CObjectPool;
        ~PTexturePool() override = default;

        XTexture::TGPULayout ConvertToGpuLayout(const XTexture& object) override
        {
            XTexture::TGPULayout gpul;

            return gpul;
        }

        void Update() override
        {
            Upload();
        }
    };
}