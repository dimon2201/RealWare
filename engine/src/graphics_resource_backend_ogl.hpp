// graphics_resource_backend_ogl.hpp

#pragma once

#include "graphics_resource_backend.hpp"
#include "types.hpp"

namespace triton
{
    class cContext;

    class cGraphicsResourceBackendOGL final : public iGraphicsResourceBackend
    {
        TRITON_OBJECT(cGraphicsResourceBackendOGL)

    public:
        explicit cGraphicsResourceBackendOGL(cContext* context);
        virtual ~cGraphicsResourceBackendOGL() override final = default;

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
        virtual cTexture* CreateTexture(
            const cVector3& size,
            cTexture::eDimension dimension,
            cTexture::eFormat format,
            const types::u8* data,
            types::s32 slot
        ) override final;
        virtual cTexture* ResizeTexture(cTexture* texture, const cVector2& size) override final;
        virtual void BindTexture(const cTexture* texture) override final;
        virtual void UnbindTexture(const cTexture* texture) override final;
        virtual void WriteTexture(
            const cTexture* texture,
            const cVector3& offset,
            const cVector2& size,
            const types::u8* data
        ) override final;
        virtual void WriteTextureToFile(const cTexture* texture, const std::string& filename) override final;
        virtual void GenerateTextureMips(const cTexture* texture) override final;
        virtual void DestroyTexture(cTexture* texture) override final;
    };
}