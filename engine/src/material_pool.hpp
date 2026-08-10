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

            STextureData& dif = *_context->GetSubsystem<XTextureSubsystem>()->GetPool()->Get(object.diffuseTexture);
            STextureData& nor = *_context->GetSubsystem<XTextureSubsystem>()->GetPool()->Get(object.normalTexture);
            STextureData& rgh = *_context->GetSubsystem<XTextureSubsystem>()->GetPool()->Get(object.roughnessTexture);
            STextureData& met = *_context->GetSubsystem<XTextureSubsystem>()->GetPool()->Get(object.metallicTexture);

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