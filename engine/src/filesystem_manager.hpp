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
            sFile(types::u8* data, types::usize byteSize) : Data(data), ByteSize(byteSize) {}

            types::u8* Data = nullptr;
            types::usize ByteSize = 0;
        };

        class mFileSystem
        {
        public:
            explicit mFileSystem(const core::cApplication* const app) : _app((core::cApplication*)app) {};
            ~mFileSystem() = default;

            sFile LoadFile(const std::string& filepath, const types::boolean isString);
            void UnloadFile(const sFile& buffer);

        private:
            core::cApplication* _app = nullptr;
        };
    }
}