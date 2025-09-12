#pragma once

#include "types.hpp"

namespace realware
{
    namespace app
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
            explicit mFileSystem(const app::cApplication* const app) : _app((app::cApplication*)app) {};
            ~mFileSystem() = default;

            sFile LoadFile(const std::string& filepath, const types::boolean isString);
            void UnloadFile(const sFile& buffer);

        private:
            app::cApplication* _app = nullptr;
        };
    }
}