// graphics_buffer_backend.hpp

#pragma once

#include "gpu_resource.hpp"
#include "backend.hpp"
#include "types.hpp"

namespace triton
{
    class cContext;

    class cBuffer : public cGPUResource
    {
        TRITON_OBJECT(cBuffer)

        friend class cGraphicsOGLBackend;

    public:
        enum class eType
        {
            NONE = 0,
            VERTEX = 1,
            INDEX = 2,
            UNIFORM = 3,
            LARGE = 4
        };

    private:
        eType _type = eType::NONE;
        types::usize _byteSize = 0;
        types::s32 _slot = -1;

    public:
        explicit cBuffer(cContext* context, types::qword instance, eType type, types::usize byteSize, types::s32 slot);
        virtual ~cBuffer() override = default;

        inline eType GetBufferType() const { return _type; }
        inline types::usize GetByteSize() const { return _byteSize; }
        inline types::s32 GetSlot() const { return _slot; }
    };

    class iGraphicsBufferBackend : public iBackend
    {
        TRITON_OBJECT(iGraphicsBufferBackend)

    public:
        explicit iGraphicsBufferBackend(cContext* context);
        virtual ~iGraphicsBufferBackend() override = default;

        virtual cBuffer* CreateBuffer(
            cBuffer::eType type,
            const types::u8* data,
            types::usize byteSize,
            types::s32 slot
        ) = 0;
        virtual void BindBuffer(const cBuffer* buffer) = 0;
        virtual void BindBufferNotVAO(const cBuffer* buffer) = 0;
        virtual void UnbindBuffer(const cBuffer* buffer) = 0;
        virtual void WriteBuffer(
            const cBuffer* buffer,
            types::usize offset,
            types::usize byteSize,
            const types::u8* data
        ) = 0;
        virtual void DestroyBuffer(cBuffer* buffer) = 0;
    };
}