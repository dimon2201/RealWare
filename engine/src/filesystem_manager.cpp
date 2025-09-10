#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include "filesystem_manager.hpp"

namespace realware
{
    using namespace core;

    namespace fs
    {
        sFile mFileSystem::LoadFile(const std::string& filepath, boolean isString)
        {
            std::ifstream inputFile(filepath, std::ios::binary);
            
            inputFile.seekg(0, std::ios::end);
            const usize byteSize = inputFile.tellg();
            inputFile.seekg(0, std::ios::beg);
            const usize databyteSize = byteSize + (isString == K_TRUE ? 1 : 0);
            
            u8* const data = (u8* const)malloc(databyteSize);
            memset(data, 0, databyteSize);
            inputFile.read((char*)&data[0], byteSize);

            sFile file(data, databyteSize);

            return file;
        }

        void mFileSystem::UnloadFile(const sFile& file)
        {
            if (file.Data == nullptr || file.ByteSize == 0)
                return;

            free(file.Data);
        }
    }
}