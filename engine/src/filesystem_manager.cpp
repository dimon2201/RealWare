// filesystem_manager.cpp

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include "application.hpp"
#include "context.hpp"
#include "filesystem_manager.hpp"
#include "memory_pool.hpp"
#include "buffer.hpp"
#include "engine.hpp"

using namespace types;

triton::cDataFile::cDataFile(cContext* context, const std::string& path, types::boolean isText)
    : iObject(context)
{
    const sCapabilities* caps = _context->GetSubsystem<cEngine>()->GetApplication()->GetCapabilities();
    auto memoryAllocator = _context->GetMemoryAllocator();

    if (_data)
        memoryAllocator->Deallocate(_data);

    std::ifstream inputFile(path, std::ios::binary);

    inputFile.seekg(0, std::ios::end);
    const usize byteSize = inputFile.tellg();
    inputFile.seekg(0, std::ios::beg);
    const usize dataByteSize = byteSize + (isText == K_TRUE ? 1 : 0);

    _data = _context->Create<cDataBuffer>(_context, dataByteSize);
    inputFile.read((char*)_data->GetData(), _data->GetByteSize());

    inputFile.close();
}

triton::cDataFile::~cDataFile()
{
    if (_data)
        _context->Destroy<cDataBuffer>(_data);
}

triton::cFileSystem::cFileSystem(cContext* context) : iObject(context) {}

triton::cDataFile* triton::cFileSystem::CreateDataFile(const std::string& path, types::boolean isText)
{
    cDataFile* file = _context->Create<cDataFile>(_context, path, isText);

    return file;
}

void triton::cFileSystem::DestroyDataFile(cDataFile* file)
{
    if (file == nullptr)
        return;

    _context->Destroy<cDataFile>(file);
}