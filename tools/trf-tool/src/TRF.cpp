// TRF.cpp

#include <fstream>
#include "TRF.hpp"
#include "log.hpp"

template <triton::EResourceFileType TResourceType>
triton::CResourceFile::CResourceFile(const std::filesystem::path& dataFilePath)
{
    std::ifstream file(dataFilePath, std::ios::binary | std::ios::ate);

    if (!file.is_open())
    {
        Print("Error: failed to open file: " + dataFilePath.string());
        return;
    }

    _dataByteSize = (types::usize)file.tellg();

    file.seekg(0, std::ios::beg);

    _data = new types::u8[_dataByteSize];

    if (!file.read((char*)_data, _dataByteSize))
    {
        delete[] _data;
        _data = nullptr;
        _dataByteSize = 0;

        Print("Error: failed to read file: " + dataFilePath.string());
    }
}

template <triton::EResourceFileType TResourceType>
triton::CResourceFile::~CResourceFile()
{
    if (_data)
        delete[] _data;
    _dataByteSize = 0;
}

template <triton::EResourceFileType TResourceType>
void triton::CResourceFile::ParseData()
{
    switch (TResourceType)
    {
        case EResourceType::Model3D:
        {
            ParseModel3DData();
            break;
        }
    }
}

template <triton::EResourceFileType TResourceType>
void triton::CResourceFile::ParseModel3DData()
{
}

template class triton::CResourceFile<triton::EResourceFileType::Model3D>;