// graphics_resource_backend.hpp

#pragma once

#include "gpu_resource.hpp"
#include "backend.hpp"
#include "math.hpp"
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
            STORAGE = 4
        };

    protected:
        eType _type = eType::NONE;
        types::usize _byteSize = 0;
        types::s32 _slot = -1;

    public:
        explicit cBuffer(cContext* context, types::qword instance, eType type, types::usize byteSize, types::s32 slot);
        virtual ~cBuffer() override = default;

        void Bind();
        void Unbind();
        void Write(types::usize byteOffset, types::u8* data, types::usize byteSize);

        inline eType GetBufferType() const { return _type; }
        inline types::usize GetByteSize() const { return _byteSize; }
        inline types::s32 GetSlot() const { return _slot; }
    };

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
            RGBA8_SRGB,
            RGB16F,
            RGBA16F,
            DEPTH_STENCIL,
            RGBA8_SRGB_MIPS
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

    class iGraphicsResourceBackend : public iBackend
    {
        TRITON_OBJECT(iGraphicsResourceBackend)

    public:
        explicit iGraphicsResourceBackend(cContext* context);
        virtual ~iGraphicsResourceBackend() override = default;

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