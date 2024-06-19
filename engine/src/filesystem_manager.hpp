#pragma once

#include "types.hpp"

namespace realware
{
    namespace core
    {
        class cApplication;

        struct sFile
        {
            sFile(u8* data, usize byteSize) : Data(data), ByteSize(byteSize) {}

            u8* Data = nullptr;
            usize ByteSize = 0;
        };

        class mFileSystem
        {

        public:
            mFileSystem(cApplication* app);
            ~mFileSystem();

            void UnloadFile(const sFile& buffer);
            sFile LoadFile(const char* filepath, boolean isString);

        private:
            cApplication* m_app;

        };
    }
}