// material.hpp

#pragma once

#include "math.hpp"
#include "handles.hpp"
#include "texture_subsystem.hpp"

namespace triton
{
    struct STextureLayout
    {
        types::u32 layer = 0;
        types::u32 _pad = 0;
        cVector2 normOffset = cVector2(0.0f);
        cVector2 normSize = cVector2(0.0f);
    };

    struct SGPUMaterialLayout
    {
        STextureLayout diffuseTextureLayout;
        STextureLayout normalTextureLayout;
        STextureLayout roughnessTextureLayout;
        STextureLayout metallicTextureLayout;
        cVector4 diffuseColor = cVector4(0.0f);
    };

    struct SMaterial
    {
        using THandle = HMaterial;
        using TGPULayout = SGPUMaterialLayout;

        HTexture diffuseTexture;
        HTexture normalTexture;
        HTexture roughnessTexture;
        HTexture metallicTexture;
        cVector4 diffuseColor = cVector4(0.0f);
    };
}