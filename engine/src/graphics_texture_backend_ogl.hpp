// graphics_texture_backend_ogl.hpp

#pragma once

#include "graphics_texture_backend.hpp"
#include "types.hpp"

namespace triton
{
    class cContext;

    class cGraphicsTextureBackendOGL final : public iGraphicsTextureBackend
    {
        TRITON_OBJECT(cGraphicsTextureBackendOGL)

    public:
        explicit cGraphicsTextureBackendOGL(cContext* context);
        virtual ~cGraphicsTextureBackendOGL() override final = default;

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