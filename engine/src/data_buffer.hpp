// data_buffer.hpp

#pragma once

#include "object.hpp"
#include "types.hpp"

namespace triton
{
    class XDataBuffer
    {
        types::u8* _data = nullptr;
        types::usize _byteSize = 0;

    public:
        explicit XDataBuffer(types::usize byteSize);
        ~XDataBuffer();

        void Write(const types::u8* data, types::usize byteSize, types::usize byteOffset);
        void Erase(types::usize byteSize, types::usize byteOffset);
        void Move(types::usize byteSize, types::usize sourceByteOffset, types::usize destinationByteOffset);

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