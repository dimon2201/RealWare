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

            STextureData& dif = _context->GetSubsystem<XTextureSubsystem>()->GetPool()->Get(object.diffuseTexture);
            STextureData& nor = _context->GetSubsystem<XTextureSubsystem>()->GetPool()->Get(object.normalTexture);
            STextureData& rgh = _context->GetSubsystem<XTextureSubsystem>()->GetPool()->Get(object.roughnessTexture);
            STextureData& met = _context->GetSubsystem<XTextureSubsystem>()->GetPool()->Get(object.metallicTexture);

            gpul.diffuseColor = object.diffuseColor;
            gpul.diffuseTextureLayout.layer = dif.layer;
            gpul.diffuseTextureLayout.normOffset = dif.normOffset;
            gpul.diffuseTextureLayout.normSize = dif.normSize;
            gpul.normalTextureLayout.layer = nor.layer;
            gpul.normalTextureLayout.normOffset = nor.normOffset;
            gpul.normalTextureLayout.normSize = nor.normSize;
            gpul.roughnessTextureLayout.layer = rgh.layer;
            gpul.roughnessTextureLayout.normOffset = rgh.normOffset;
            gpul.roughnessTextureLayout.normSize = rgh.normSize;
            gpul.metallicTextureLayout.layer = met.layer;
            gpul.metallicTextureLayout.normOffset = met.normOffset;
            gpul.metallicTextureLayout.normSize = met.normSize;

            return gml;
        }

        void Update() override
        {
            Upload();
        }
    };
}