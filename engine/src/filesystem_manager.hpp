// filesystem_manager.hpp

#pragma once

#include "object.hpp"
#include "data_buffer.hpp"
#include "types.hpp"

namespace triton
{
    class cDataFile : public iObject
    {
        TRITON_OBJECT(cDataFile)

        XDataBuffer* _data = nullptr;

    public:
        explicit cDataFile(cContext* context, const std::string& path, types::boolean isText);
        virtual ~cDataFile() override final;

        inline void* GetData() const;
        inline XDataBuffer* GetBuffer() const { return _data; }
    };

    class cFileSystem : public iObject
    {
        TRITON_OBJECT(cFileSystem)

    public:
        explicit cFileSystem(cContext* context);
        ~cFileSystem() = default;

        cDataFile* CreateDataFile(const std::string& path, types::boolean isText);
        std::string TextFileToString(const std::string& path);
        types::usize BinFileToArray(const std::string& path, types::u8* array, types::usize offset, types::usize maxByteSize);
        types::usize TellFileByteSize(const std::string& path);
        void DestroyDataFile(cDataFile* buffer);
    };
}

void* triton::cDataFile::GetData() const
{
    if (_data == nullptr)
        return nullptr;
    else
        return _data->GetData();
}