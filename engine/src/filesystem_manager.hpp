#pragma once

#include "types.hpp"

namespace realware
{
    namespace core
    {
        struct sFile
        {
            sFile(u8* data, usize byteSize) : Data(data), ByteSize(byteSize) {}

            u8* Data = nullptr;
            usize ByteSize = 0;
        };

        class mFileSystem
        {

        public:
            mFileSystem() {}
            ~mFileSystem() {}

            void Init();
            void Free();
            void DeleteFile(const sFile& buffer);
            sFile LoadFile(const char* filepath, boolean isString);
        };
    }
}