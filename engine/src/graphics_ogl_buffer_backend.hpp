// graphics_ogl_buffer_backend.hpp

#pragma once

#include "graphics_buffer_backend.hpp"
#include "types.hpp"

namespace triton
{
    class cContext;

    class cGraphicsOGLBufferBackend final : public iGraphicsBufferBackend
    {
        TRITON_OBJECT(cGraphicsOGLBufferBackend)

    public:
        explicit cGraphicsOGLBufferBackend(cContext* context);
        virtual ~cGraphicsOGLBufferBackend() override final = default;

        virtual cBuffer* CreateBuffer(
            cBuffer::eType type,
            const types::u8* data,
            types::usize byteSize,
            types::s32 slot
        ) override final;
        virtual void BindBuffer(const cBuffer* buffer) override final;
		virtual void BindBufferNotVAO(const cBuffer* buffer) override final;
        virtual void UnbindBuffer(const cBuffer* buffer) override final;
        virtual void WriteBuffer(
            const cBuffer* buffer,
            types::usize offset,
            types::usize byteSize,
            const types::u8* data
        ) override final;
        virtual void DestroyBuffer(cBuffer* buffer) override final;
    };
}