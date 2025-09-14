#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include "filesystem_manager.hpp"

namespace realware
{
    using namespace types;

    namespace fs
    {
        sFile* mFileSystem::CreateDataFile(const std::string& filepath, boolean isString)
        {
            std::ifstream inputFile(filepath, std::ios::binary);
            
            inputFile.seekg(0, std::ios::end);
            const usize byteSize = inputFile.tellg();
            inputFile.seekg(0, std::ios::beg);
            const usize databyteSize = byteSize + (isString == K_TRUE ? 1 : 0);
            
            u8* const data = (u8* const)malloc(databyteSize);
            memset(data, 0, databyteSize);
            inputFile.read((char*)&data[0], byteSize);

            sFile* file = new sFile;
            file->Data = data;
            file->DataByteSize = databyteSize;

            return file;
        }

        void mFileSystem::DestroyDataFile(sFile* file)
        {
            void* fileData = file->Data;

            if (fileData == nullptr || file->DataByteSize == 0)
                return;

            free(fileData);
        }
    }
}