// material_uploader.hpp

#pragma once

#include "object.hpp"

namespace triton
{
    class cContext;
    class cBuffer;
    struct SMaterial;

    struct SMaterialTextureLayout
    {
        types::u32 atlasLayer = 0;
        types::u32 _pad = 0;
        cVector2 atlasNormOffset = cVector2(0.0f);
        cVector2 atlasNormSize = cVector2(0.0f);
    };

    struct SMaterialLayout
    {
        SMaterialTextureLayout diffuse;
        types::u32 _pad[2];
        cVector4 diffuseColor = cVector4(0.0f);
    };

    class XMaterialUploader final : public iObject
    {
        TRITON_OBJECT(XMaterialUploader)

        SMaterialLayout* _buffer = nullptr;

    public:
        explicit XMaterialUploader(cContext* context);
        ~XMaterialUploader() override;

        void Set(types::usize index, const SMaterial& material);
        void Upload(cBuffer* materialBuffer);
    };
}