// filesystem_manager.cpp

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include "application.hpp"
#include "context.hpp"
#include "filesystem_manager.hpp"
#include "memory_pool.hpp"
#include "data_buffer.hpp"
#include "engine.hpp"
#include "object_allocator.hpp"

using namespace types;

triton::cDataFile::cDataFile(cContext* context, const std::string& path, types::boolean isText) : _context(context)
{
    if (_data)
        CObjectAllocator::Deallocate(_data);

    std::ifstream inputFile(path, std::ios::binary);
    if (!inputFile.is_open())
    {
        _bExists = K_FALSE;
        Print("Error: file '" + path + "' does not exist");

        return;
    }
    else
    {
        _bExists = K_TRUE;
    }

    inputFile.seekg(0, std::ios::end);
    const usize byteSize = inputFile.tellg();
    inputFile.seekg(0, std::ios::beg);
    const usize dataByteSize = byteSize + (isText == K_TRUE ? 1 : 0);

    _data = _context->Create<XDataBuffer>(dataByteSize);
    inputFile.read((char*)_data->GetData(), _data->GetByteSize());

    inputFile.close();
}

triton::cDataFile::~cDataFile()
{
    if (_data)
        _context->Destroy<XDataBuffer>(_data);
}

triton::CFileSystem::CFileSystem(cContext* context) : CSubsystem(context) {}

triton::cDataFile* triton::CFileSystem::CreateDataFile(const std::string& path, types::boolean isText)
{
    cDataFile* file = _context->Create<cDataFile>(_context, path, isText);

    return file;
}

std::string triton::CFileSystem::TextFileToString(const std::string& path)
{
    cDataFile* file = _context->Create<cDataFile>(_context, path, K_TRUE);
    std::string str = "";
    for (usize i = 0; i < file->GetBuffer()->GetByteSize(); i++)
        str.push_back(((u8*)file->GetData())[i]);
    _context->Destroy<cDataFile>(file);
    
    return str;
}

usize triton::CFileSystem::BinFileToArray(const std::string& path, types::u8* array, usize offset, usize maxByteSize)
{
    cDataFile* file = _context->Create<cDataFile>(_context, path, K_FALSE);
    if (offset + file->GetBuffer()->GetByteSize() > maxByteSize)
    {
        Print("Error: binary file size exceeds array size");
        return 0;
    }
    usize fileByteSize = file->GetBuffer()->GetByteSize();
    for (usize i = 0; i < fileByteSize; i++)
        array[offset + i] = file->GetBuffer()->GetData()[i];
    _context->Destroy<cDataFile>(file);

    return fileByteSize;
}

usize triton::CFileSystem::TellFileByteSize(const std::string& path)
{
    std::ifstream inputFile(path, std::ios::binary);
    inputFile.seekg(0, std::ios::end);
    const usize byteSize = inputFile.tellg();
    inputFile.seekg(0, std::ios::beg);
    inputFile.close();

    return byteSize;
}

void triton::CFileSystem::DestroyDataFile(cDataFile* file)
{
    if (file == nullptr)
        return;

    _context->Destroy<cDataFile>(file);
}