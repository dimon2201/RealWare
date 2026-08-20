// filesystem_manager.hpp

#pragma once

#include "object.hpp"
#include "data_buffer.hpp"
#include "subsystem.hpp"
#include "types.hpp"

namespace triton
{
    class cContext;

    class cDataFile
    {
        cContext* _context = nullptr;
        types::boolean _bExists = types::K_FALSE;
        XDataBuffer* _data = nullptr;

    public:
        explicit cDataFile(cContext* context, const std::string& path, types::boolean isText);
        ~cDataFile();

        inline void* GetData() const;
        inline XDataBuffer* GetBuffer() const { return _data; }

        inline types::boolean Exists() const
        {
            return _bExists;
        }
    };

    class CFileSystem : public CSubsystem
    {
        TRITON_CLASS_NAME(CFileSystem)

    public:
        explicit CFileSystem(cContext* context);
        ~CFileSystem() override = default;

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