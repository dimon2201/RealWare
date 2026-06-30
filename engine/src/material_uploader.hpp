// material_uploader.hpp

#pragma once

#include "object.hpp"

namespace triton
{
    class cBuffer;

    class XMaterialUploader final : public iObject
    {
        TRITON_OBJECT(XMaterialUploader)

        cBuffer* _materialBuffer = nullptr;
    };
}