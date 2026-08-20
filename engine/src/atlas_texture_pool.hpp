// atlas_texture_pool.hpp

#pragma once

#include "object_pool.hpp"

namespace triton
{
    class XAtlasTexturePool : public XObjectPool<XAtlasTexture>
    {
        TRITON_OBJECT(XAtlasTexturePool)

    public:
        using XObjectPool<XAtlasTexture>::XObjectPool;
        ~XAtlasTexturePool() override = default;

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