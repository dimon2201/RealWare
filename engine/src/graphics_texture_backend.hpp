// graphics_texture_backend.hpp

#pragma once

#include "gpu_resource.hpp"
#include "backend.hpp"
#include "math.hpp"
#include "types.hpp"

namespace triton
{
    class cContext;

    class cTexture : public cGPUResource
    {
        TRITON_OBJECT(cTexture)

    public:
        enum class eDimension
        {
            NONE = 0,
            TEXTURE_2D,
            TEXTURE_2D_ARRAY
        };

        enum class eFormat
        {
            NONE = 0,
            R8,
            R8F,
            RGBA8,
            RGB16F,
            RGBA16F,
            DEPTH_STENCIL,
            RGBA8_MIPS
        };

    private:
        cVector3 _size = cVector3(0.0f);
        eDimension _dimension = eDimension::NONE;
        eFormat _format = eFormat::NONE;
        types::s32 _slot = -1;

    public:
        explicit cTexture(
            cContext* context,
            types::qword instance,
            const cVector3& size,
            eDimension dimension,
            eFormat format,
            types::s32 slot
        );
        virtual ~cTexture() override = default;

        inline types::usize GetWidth() const { return _size.GetX(); }
        inline types::usize GetHeight() const { return _size.GetY(); }
        inline types::usize GetDepth() const { return _size.GetZ(); }
        inline eDimension GetDimension() const { return _dimension; }
        inline eFormat GetFormat() const { return _format; }
        inline types::s32 GetSlot() const { return _slot; }
        inline void SetSlot(types::s32 slot) { _slot = slot; }
    };

    class iGraphicsTextureBackend : public iBackend
    {
        TRITON_OBJECT(iGraphicsTextureBackend)

    public:
        explicit iGraphicsTextureBackend(cContext* context);
        virtual ~iGraphicsTextureBackend() override = default;

        virtual cTexture* CreateTexture(
            const cVector3& size,
            cTexture::eDimension dimension,
            cTexture::eFormat format,
            const types::u8* data,
            types::s32 slot
        ) = 0;
        virtual cTexture* ResizeTexture(cTexture* texture, const cVector2& size) = 0;
        virtual void BindTexture(const cTexture* texture) = 0;
        virtual void UnbindTexture(const cTexture* texture) = 0;
        virtual void WriteTexture(
            const cTexture* texture,
            const cVector3& offset,
            const cVector2& size,
            const types::u8* data
        ) = 0;
        virtual void WriteTextureToFile(const cTexture* texture, const std::string& filename) = 0;
        virtual void GenerateTextureMips(const cTexture* texture) = 0;
        virtual void DestroyTexture(cTexture* texture) = 0;
    };
}