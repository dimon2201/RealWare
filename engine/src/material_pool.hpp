// material_pool.hpp

#pragma once

#include "material_data.hpp"
#include "object_pool.hpp"
#include "atlas_texture_pool.hpp"

namespace triton
{
    class XMaterialPool : public XObjectPool<SMaterialData>
    {
        TRITON_OBJECT(XMaterialPool)

    public:
        using XObjectPool<SMaterialData>::XObjectPool;
        ~XMaterialPool() override = default;

        SMaterialData::TGPULayout ConvertToGpuLayout(const SMaterialData& object) override
        {
            SMaterialData::TGPULayout gpul;

            auto difRes = _context->GetPool<XAtlasTexturePool>()->Get(object.diffuseTexture);
            auto norRes = _context->GetPool<XAtlasTexturePool>()->Get(object.normalTexture);
            auto rghRes = _context->GetPool<XAtlasTexturePool>()->Get(object.roughnessTexture);
            auto metRes = _context->GetPool<XAtlasTexturePool>()->Get(object.metallicTexture);

            gpul.diffuseColor = object.diffuseColor;
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

            return gpul;
        }

        void Update() override
        {
            Upload();
        }
    };
}