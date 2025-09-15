#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include "application.hpp"
#include "filesystem_manager.hpp"
#include "memory_pool.hpp"

using namespace types;

namespace realware
{
    using namespace app;
    using namespace utils;

    namespace fs
    {
        sFile* mFileSystem::CreateDataFile(const std::string& filepath, types::boolean isString)
        {
            std::ifstream inputFile(filepath, std::ios::binary);
            
            inputFile.seekg(0, std::ios::end);
            const usize byteSize = inputFile.tellg();
            inputFile.seekg(0, std::ios::beg);
            const usize databyteSize = byteSize + (isString == K_TRUE ? 1 : 0);
            
            u8* const data = (u8* const)malloc(databyteSize);
            memset(data, 0, databyteSize);
            inputFile.read((char*)&data[0], byteSize);

            sFile* pFile = (sFile*)_app->GetMemoryPool()->Allocate(sizeof(sFile));
            sFile* file = new (pFile) sFile;

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