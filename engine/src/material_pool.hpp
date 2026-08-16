// material_pool.hpp

#pragma once

#include "material_data.hpp"
#include "object_pool_base.hpp"
#include "texture_pool.hpp"

namespace triton
{
    class XMaterialPool : public XObjectPoolBase<SMaterialData>
    {
    public:
        using XObjectPoolBase<SMaterialData>::XObjectPoolBase;
        ~XMaterialPool() override = default;

        SMaterialData::TGPULayout ConvertToGpuLayout(const SMaterialData& object) override
        {
            SMaterialData::TGPULayout gpul;

            auto difRes = _context->GetSubsystem<XTextureSubsystem>()->GetPool()->Get(object.diffuseTexture);
            auto norRes = _context->GetSubsystem<XTextureSubsystem>()->GetPool()->Get(object.normalTexture);
            auto rghRes = _context->GetSubsystem<XTextureSubsystem>()->GetPool()->Get(object.roughnessTexture);
            auto metRes = _context->GetSubsystem<XTextureSubsystem>()->GetPool()->Get(object.metallicTexture);

            STextureData empty;
            STextureData& dif = empty;
            STextureData& nor = empty;
            STextureData& rgh = empty;
            STextureData& met = empty;
            if (difRes.has_value()) dif = *difRes;
            if (norRes.has_value()) nor = *norRes;
            if (rghRes.has_value()) rgh = *rghRes;
            if (metRes.has_value()) met = *metRes;

            gpul.diffuseColor = object.diffuseColor;
            gpul.diffuseTextureLayout.layer = dif.zAtlasLayer;
            gpul.diffuseTextureLayout.normOffset = dif.offsetNorm;
            gpul.diffuseTextureLayout.normSize = dif.sizeNorm;
            gpul.normalTextureLayout.layer = nor.zAtlasLayer;
            gpul.normalTextureLayout.normOffset = nor.offsetNorm;
            gpul.normalTextureLayout.normSize = nor.sizeNorm;
            gpul.roughnessTextureLayout.layer = rgh.zAtlasLayer;
            gpul.roughnessTextureLayout.normOffset = rgh.offsetNorm;
            gpul.roughnessTextureLayout.normSize = rgh.sizeNorm;
            gpul.metallicTextureLayout.layer = met.zAtlasLayer;
            gpul.metallicTextureLayout.normOffset = met.offsetNorm;
            gpul.metallicTextureLayout.normSize = met.sizeNorm;

            return gpul;
        }

        void Update() override
        {
            Upload();
        }
    };
}