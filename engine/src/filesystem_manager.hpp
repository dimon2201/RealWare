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
            types::u8* Data = nullptr;
            types::usize DataByteSize = 0;
        };

        class mFileSystem
        {
        public:
            explicit mFileSystem(const app::cApplication* const app);
            ~mFileSystem() = default;

            sFile* CreateDataFile(const std::string& filepath, const types::boolean isString);
            void DestroyDataFile(sFile* buffer);

        private:
            app::cApplication* _app = nullptr;
        };
    }
}