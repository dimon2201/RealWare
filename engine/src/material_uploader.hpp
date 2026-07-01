// material_uploader.hpp

#pragma once

#include "object.hpp"
#include "math.hpp"

namespace triton
{
    class cContext;
    class cBuffer;
    struct SMaterial;
    class XTextureSubsystem;

    struct STextureLayout
    {
        types::u32 layer = 0;
        types::u32 _pad = 0;
        cVector2 normOffset = cVector2(0.0f);
        cVector2 normSize = cVector2(0.0f);
    };

    struct SMaterialLayout
    {
        STextureLayout diffuseTextureLayout;
        STextureLayout normalTextureLayout;
        cVector4 diffuseColor = cVector4(0.0f);
    };

    class XMaterialUploader final
    {
        cContext* _context = nullptr;
        types::boolean _bIsDirty = types::K_FALSE;
        SMaterialLayout* _stagingBuffer = nullptr;

    public:
        explicit XMaterialUploader(cContext* context);
        ~XMaterialUploader();

        void Set(XTextureSubsystem* textureSubsystem, types::usize index, const SMaterial& material);
        void MarkDirty();
        void Upload(cBuffer* materialBuffer, types::usize byteSizeToUpload);
    };
}