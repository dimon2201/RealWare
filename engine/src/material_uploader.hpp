// material_uploader.hpp

#pragma once

#include "object.hpp"
#include "math.hpp"

namespace triton
{
    class cContext;
    class cBuffer;
    struct SMaterial;
    struct SGPUMaterialLayout;
    class XTextureSubsystem;

    class XMaterialUploader final
    {
        cContext* _context = nullptr;
        types::boolean _bIsDirty = types::K_FALSE;
        SGPUMaterialLayout* _stagingBuffer = nullptr;

    public:
        explicit XMaterialUploader(cContext* context);
        ~XMaterialUploader();

        void Set(XTextureSubsystem* textureSubsystem, types::usize index, const SMaterial& material);
        void MarkDirty();
        void Upload(cBuffer* materialBuffer, types::usize byteSizeToUpload);
    };
}