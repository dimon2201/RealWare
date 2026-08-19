// material_data.hpp

#pragma once

#include "math.hpp"
#include "atlas_texture.hpp"

namespace triton
{
    struct STextureLayout
    {
        types::u32 layer = 0;
        types::u32 _pad = 0;
        cVector2 normOffset = cVector2(0.0f);
        cVector2 normSize = cVector2(0.0f);
    };

    struct SMaterialData
    {
        struct THandle : public SHandle {};

        struct TGPULayout
        {
            STextureLayout diffuseTextureLayout;
            STextureLayout normalTextureLayout;
            STextureLayout roughnessTextureLayout;
            STextureLayout metallicTextureLayout;
            cVector4 diffuseColor = cVector4(0.0f);
        };

        XAtlasTexture::THandle diffuseTexture;
        XAtlasTexture::THandle normalTexture;
        XAtlasTexture::THandle roughnessTexture;
        XAtlasTexture::THandle metallicTexture;
        cVector4 diffuseColor = cVector4(0.0f);
    };
}