// data_buffer.hpp

#pragma once

#include "object.hpp"
#include "types.hpp"

namespace triton
{
    class XDataBuffer : public iObject
    {
        TRITON_OBJECT(XDataBuffer)

        types::u8* _data = nullptr;
        types::usize _byteSize = 0;

    public:
        explicit XDataBuffer(cContext* context, types::usize byteSize);
        virtual ~XDataBuffer() override;

        void Write(const types::u8* data, types::usize byteSize, types::usize byteOffset);

        inline types::u8* GetData() const
        {
            return _data;
        }

        inline types::usize GetByteSize() const
        {
            return _byteSize;
        }
    };
}