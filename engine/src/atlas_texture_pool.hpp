// atlas_texture_pool.hpp

#pragma once

#include "object_pool_base.hpp"

namespace triton
{
    class XAtlasTexturePool : public XObjectPoolBase<XAtlasTexture>
    {
        TRITON_OBJECT(XAtlasTexturePool)

    public:
        using XObjectPoolBase<XAtlasTexture>::XObjectPoolBase;
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