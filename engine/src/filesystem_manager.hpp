// filesystem_manager.hpp

#pragma once

#include "object.hpp"
#include "buffer.hpp"
#include "types.hpp"

namespace triton
{
    class cDataFile : public iObject
    {
        TRITON_OBJECT(cDataFile)

    public:
        explicit cDataFile(cContext* context, const std::string& path, types::boolean isText);
        virtual ~cDataFile() override final;

        inline void* GetData() const;
        inline cDataBuffer* GetBuffer() const { return _data; }

    private:
        cDataBuffer* _data = nullptr;
    };

    class cFileSystem : public iObject
    {
        TRITON_OBJECT(cFileSystem)

    public:
        explicit cFileSystem(cContext* context);
        ~cFileSystem() = default;

        cDataFile* CreateDataFile(const std::string& path, types::boolean isText);
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