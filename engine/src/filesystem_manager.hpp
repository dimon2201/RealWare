#pragma once

#include "types.hpp"

namespace realware
{
    namespace core
    {
        class cApplication;
    }

    namespace fs
    {
        struct sFile
        {
            sFile(core::u8* data, core::usize byteSize) : Data(data), ByteSize(byteSize) {}

            core::u8* Data = nullptr;
            core::usize ByteSize = 0;
        };

        class mFileSystem
        {
        public:
            explicit mFileSystem(const core::cApplication* const app) : _app((core::cApplication*)app) {};
            ~mFileSystem() = default;

            sFile LoadFile(const std::string& filepath, const core::boolean isString);
            void UnloadFile(const sFile& buffer);

        private:
            core::cApplication* _app = nullptr;
        };
    }
}