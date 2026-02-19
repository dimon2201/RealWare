// graphics_buffer_backend_ogl.hpp

#pragma once

#include "graphics_buffer_backend.hpp"
#include "types.hpp"

namespace triton
{
    class cContext;

    class cGraphicsBufferBackendOGL final : public iGraphicsBufferBackend
    {
        TRITON_OBJECT(cGraphicsBufferBackendOGL)

    public:
        explicit cGraphicsBufferBackendOGL(cContext* context);
        virtual ~cGraphicsBufferBackendOGL() override final = default;

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
    };
}