// buffer.hpp

#pragma once

#include "object.hpp"
#include "types.hpp"

namespace triton
{
    class cDataBuffer : public iObject
    {
        TRITON_OBJECT(cDataBuffer)

        types::u8* _data = nullptr;
        types::usize _byteSize = 0;

    public:
        explicit cDataBuffer(cContext* context, types::usize byteSize);
        virtual ~cDataBuffer() override;

        inline types::u8* GetData() const { return _data; }
        inline types::usize GetByteSize() const { return _byteSize; }
    };
}