// material.hpp

#pragma once

#include "object.hpp"
#include "math.hpp"
#include "atlas_texture.hpp"

namespace triton
{
    struct STextureLayout final
    {
        types::u32 layer = 0;
        types::u32 _pad = 0;
        cVector2 normOffset = cVector2(0.0f);
        cVector2 normSize = cVector2(0.0f);
    };

    class XMaterial : public iObject
    {
        TRITON_OBJECT(XMaterial)

        XAtlasTexture::THandle  _diffuseTexture;
        XAtlasTexture::THandle  _normalTexture;
        XAtlasTexture::THandle  _roughnessTexture;
        XAtlasTexture::THandle  _metallicTexture;
        cVector4                _diffuseColor = cVector4(1.0f);
        cVector4                _specularColor = cVector4(1.0f);
        types::f32              _shininess = 0.0f;

    public:
        explicit XMaterial(cContext* context, types::s32 poolIndex) : iObject(context, poolIndex) {}

        explicit XMaterial(
            cContext* context,
            types::s32 poolIndex,
            const XAtlasTexture::THandle& diffuseTexture,
            const XAtlasTexture::THandle& normalTexture,
            const XAtlasTexture::THandle& roughnessTexture,
            const XAtlasTexture::THandle& metallicTexture,
            const cVector4& diffuseColor,
            const cVector4& specularColor,
            types::f32 shininess
        );

        ~XMaterial() override = default;

        inline const XAtlasTexture::THandle& GetDiffuseTexture() const { return _diffuseTexture; }

        inline const XAtlasTexture::THandle& GetNormalTexture() const { return _normalTexture; }

        inline const XAtlasTexture::THandle& GetRoughnessTexture() const { return _roughnessTexture; }

        inline const XAtlasTexture::THandle& GetMetallicTexture() const { return _metallicTexture; }

        inline const cVector4& GetDiffuseColor() const { return _diffuseColor; }

        inline const cVector4& GetSpecularColor() const { return _specularColor; }

        inline types::f32 GetShininess() const { return _shininess; }

        struct THandle : public SHandle {};

        struct TGPULayout
        {
            STextureLayout diffuseTextureLayout;
            STextureLayout normalTextureLayout;
            STextureLayout roughnessTextureLayout;
            STextureLayout metallicTextureLayout;
            cVector4 diffuseColor = cVector4(0.0f);
            cVector4 specularColor = cVector4(0.0f);
            types::f32 shininess = 0.0f;
        };
    };
}