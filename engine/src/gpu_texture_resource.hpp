// gpu_texture.hpp

#pragma once

#include "math.hpp"
#include "texture_formats.hpp"
#include "texture_dimensions.hpp"
#include "gpu_resource.hpp"

namespace triton
{
    class CGPUTextureResource : public CGPUResource
    {
        types::qword _sampler = 0;
        cVector3 _size = cVector3(0.0f);
        ETextureFormat _format = ETextureFormat::Unknown;
        types::dword _usageMask = 0;
        ETextureDimension _dimension = ETextureDimension::Unknown;
        types::s32 _slot = -1;

    public:
        explicit CGPUTextureResource(
            types::qword instance,
            types::qword viewInstance,
            types::qword sampler,
            ETextureFormat format,
            types::dword usageMask,
            ETextureDimension dimension,
            const cVector3& size,
            types::s32 slot
        ) : CGPUResource(instance, viewInstance),
            _sampler(sampler),
            _format(format),
            _usageMask(usageMask),
            _dimension(dimension),
            _size(size),
            _slot(slot) {}
        ~CGPUTextureResource() override = default;

        static CGPUTextureResource Invalid()
        {
            return CGPUTextureResource(
                0,
                0,
                0,
                ETextureFormat::Unknown,
                0,
                ETextureDimension::Unknown,
                cVector3(0.0f),
                0
            );
        }

        inline types::qword GetSampler() const { return _sampler; }

        inline types::usize GetWidth() const { return _size.GetX(); }

        inline types::usize GetHeight() const { return _size.GetY(); }

        inline types::usize GetDepth() const { return _size.GetZ(); }

        inline ETextureFormat GetFormat() const { return _format; }

        inline types::dword GetUsageMask() const { return _usageMask; }

        inline ETextureDimension GetDimension() const { return _dimension; }

        inline types::s32 GetSlot() const { return _slot; }
    };
}