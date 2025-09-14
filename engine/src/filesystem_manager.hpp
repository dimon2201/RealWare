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
        class cFile
        {
        public:
            cFile(types::u8* data, types::usize byteSize) : _data(data), _byteSize(byteSize) {}
            ~cFile() = delete;

            inline types::u8* GetData() const { return _data; }
            inline types::usize GetByteSize() const { return _byteSize; }

        private:
            types::u8* _data = nullptr;
            types::usize _byteSize = 0;
        };

        class mFileSystem
        {
        public:
            explicit mFileSystem(const app::cApplication* const app) : _app((app::cApplication*)app) {};
            ~mFileSystem() = default;

            cFile* CreateDataFile(const std::string& filepath, const types::boolean isString);
            void DestroyDataFile(cFile* buffer);

        private:
            app::cApplication* _app = nullptr;
        };
    }
}