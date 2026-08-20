// atlas_texture_pool.hpp

#pragma once

#include "object_pool.hpp"

namespace triton
{
    class CAtlasTexturePool : public CObjectPool<XAtlasTexture>
    {
        TRITON_CLASS_NAME(CAtlasTexturePool)

    public:
        using CObjectPool<XAtlasTexture>::CObjectPool;
        ~CAtlasTexturePool() override = default;

        XAtlasTexture::TGPULayout ConvertToGpuLayout(const XAtlasTexture& object) override
        {
            XAtlasTexture::TGPULayout gpul;

            return gpul;
        }

        void Update() override
        {
            Upload();
        }
    };
}