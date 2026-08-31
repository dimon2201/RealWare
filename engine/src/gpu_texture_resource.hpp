// gpu_texture.hpp

#pragma once

#include "math.hpp"
#include "texture_formats.hpp"
#include "texture_dimensions.hpp"
#include "gpu_resource.hpp"
#include "gpu_buffer_resource.hpp"

namespace triton
{
    class CGPUTextureResource : public CGPUResource
    {
        types::qword _sampler = 0;
        types::qword _deviceMemory = 0;
        cVector3 _size = cVector3(0.0f);
        ETextureFormat _format = ETextureFormat::Unknown;
        types::dword _usageMask = 0;
        ETextureDimension _dimension = ETextureDimension::Unknown;
        types::s32 _slot = -1;
        const CGPUBufferResource* _stagingBuffer = nullptr;

    public:
        explicit CGPUTextureResource(
            types::qword instance,
            types::qword viewInstance,
            types::qword sampler,
            types::qword deviceMemory,
            ETextureFormat format,
            types::dword usageMask,
            ETextureDimension dimension,
            const cVector3& size,
            types::s32 slot,
            const CGPUBufferResource* stagingBuffer
        ) : CGPUResource(instance, viewInstance),
            _sampler(sampler),
            _deviceMemory(deviceMemory),
            _format(format),
            _usageMask(usageMask),
            _dimension(dimension),
            _size(size),
            _slot(slot),
            _stagingBuffer(stagingBuffer) {}
        ~CGPUTextureResource() override = default;

        static CGPUTextureResource Invalid()
        {
            return CGPUTextureResource(
                0,
                0,
                0,
                0,
                ETextureFormat::Unknown,
                0,
                ETextureDimension::Unknown,
                cVector3(0.0f),
                0,
                nullptr
            );
        }

        inline types::qword GetSampler() const { return _sampler; }

        inline types::qword GetDeviceMemory() const { return _deviceMemory; }

        inline const cVector3& GetSize() const { return _size; }

        inline types::usize GetWidth() const { return _size.GetX(); }

        inline types::usize GetHeight() const { return _size.GetY(); }

        inline types::usize GetDepth() const { return _size.GetZ(); }

        inline ETextureFormat GetFormat() const { return _format; }

        inline types::dword GetUsageMask() const { return _usageMask; }

        inline ETextureDimension GetDimension() const { return _dimension; }

        inline types::s32 GetSlot() const { return _slot; }

        inline const CGPUBufferResource* GetStagingBuffer() const { return _stagingBuffer; }
    };
}