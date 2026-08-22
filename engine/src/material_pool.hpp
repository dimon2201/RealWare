// material_pool.hpp

#pragma once

#include "material.hpp"
#include "object_pool.hpp"
#include "atlas_texture_pool.hpp"

namespace triton
{
    class CMaterialPool : public CObjectPool<XMaterial>
    {
        TRITON_CLASS_NAME(CMaterialPool)

    public:
        using CObjectPool<XMaterial>::CObjectPool;
        ~CMaterialPool() override = default;

        XMaterial::TGPULayout ConvertToGpuLayout(const XMaterial& object) override
        {
            XMaterial::TGPULayout gpul;

            auto difRes = _context->GetPool<CAtlasTexturePool>()->Get(object.GetDiffuseTexture());
            auto norRes = _context->GetPool<CAtlasTexturePool>()->Get(object.GetNormalTexture());
            auto rghRes = _context->GetPool<CAtlasTexturePool>()->Get(object.GetRoughnessTexture());
            auto metRes = _context->GetPool<CAtlasTexturePool>()->Get(object.GetMetallicTexture());

            if (difRes.has_value())
            {
                XAtlasTexture& dif = *difRes;
                gpul.diffuseTextureLayout.layer = dif.GetAtlasZLayer();
                gpul.diffuseTextureLayout.normOffset = dif.GetOffsetNorm();
                gpul.diffuseTextureLayout.normSize = dif.GetSizeNorm();
            }
            if (norRes.has_value())
            {
                XAtlasTexture& nor = *norRes;
                gpul.normalTextureLayout.layer = nor.GetAtlasZLayer();
                gpul.normalTextureLayout.normOffset = nor.GetOffsetNorm();
                gpul.normalTextureLayout.normSize = nor.GetSizeNorm();
            }
            if (rghRes.has_value())
            {
                XAtlasTexture& rgh = *rghRes;
                gpul.roughnessTextureLayout.layer = rgh.GetAtlasZLayer();
                gpul.roughnessTextureLayout.normOffset = rgh.GetOffsetNorm();
                gpul.roughnessTextureLayout.normSize = rgh.GetSizeNorm();
            }
            if (metRes.has_value())
            {
                XAtlasTexture& met = *metRes;
                gpul.metallicTextureLayout.layer = met.GetAtlasZLayer();
                gpul.metallicTextureLayout.normOffset = met.GetOffsetNorm();
                gpul.metallicTextureLayout.normSize = met.GetSizeNorm();
            }
            gpul.diffuseColor = object.GetDiffuseColor();
            gpul.specularColor = object.GetSpecularColor();
            gpul.shininess = object.GetShininess();

            return gpul;
        }

        void Update() override
        {
            Upload();
        }
    };
}