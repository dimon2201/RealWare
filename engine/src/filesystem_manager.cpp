#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include "filesystem_manager.hpp"

namespace realware
{
    namespace core
    {
        void mFileSystem::Init()
        {
        }

        void mFileSystem::Free()
        {
        }

        void mFileSystem::DeleteFile(const sFile& file)
        {
            if (file.Data == nullptr || file.ByteSize == 0) {
                return;
            }

            free(file.Data);
        }

        sFile mFileSystem::LoadFile(const char* filepath, boolean isString)
        {
            std::ifstream inputFile(filepath, std::ios::binary);
            inputFile.seekg(0, std::ios::end);
            usize byteSize = inputFile.tellg();
            inputFile.seekg(0, std::ios::beg);
            usize databyteSize = byteSize + (isString == K_TRUE ? 1 : 0);
            u8* data = (u8*)malloc(databyteSize);
            memset(data, 0, databyteSize);
            inputFile.read((char*)&data[0], byteSize);

            sFile file(data, databyteSize);

            return file;
        }
    }
}