// gpu_texture.hpp

#pragma once

#include "math.hpp"
#include "texture_formats.hpp"
#include "texture_dimensions.hpp"
#include "gpu_resource.hpp"

namespace triton
{
    class CGPUTexture : public cGPUResource
    {
        TRITON_OBJECT(CGPUTexture)

        cVector3 _size = cVector3(0.0f);
        ETextureDimension _dimension = ETextureDimension::Unknown;
        ETextureFormat _format = ETextureFormat::Unknown;
        types::s32 _slot = -1;

    public:
        explicit CGPUTexture() = default;
        explicit CGPUTexture(
            cContext* context,
            types::qword instance,
            types::qword viewInstance,
            const cVector3& size,
            ETextureDimension dimension,
            ETextureFormat format,
            types::s32 slot
        ) : cGPUResource(context, instance, viewInstance),
            _size(size),
            _dimension(dimension),
            _format(format),
            _slot(slot) {
        }
        ~CGPUTexture() override = default;

        inline types::usize GetWidth() const { return _size.GetX(); }

        inline types::usize GetHeight() const { return _size.GetY(); }

        inline types::usize GetDepth() const { return _size.GetZ(); }

        inline ETextureDimension GetDimension() const { return _dimension; }

        inline ETextureFormat GetFormat() const { return _format; }

        inline types::s32 GetSlot() const { return _slot; }
    };
}