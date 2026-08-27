// gpu_buffer_resource.hpp

#pragma once

#include "gpu_resource.hpp"
#include "gpu_buffer_types.hpp"
#include "types.hpp"

namespace triton
{
    class CGPUBufferResource : public CGPUResource
    {
        types::qword _deviceMemory = 0;
        EGPUBufferType _type = EGPUBufferType::Unknown;
        types::usize _byteSize = 0;
        types::s32 _slot = -1;

    public:
        explicit CGPUBufferResource(
            types::qword instance,
            types::qword viewInstance,
            types::qword deviceMemory,
            EGPUBufferType type,
            types::usize byteSize,
            types::s32 slot
        ) :
            CGPUResource(instance, viewInstance),
            _deviceMemory(deviceMemory),
            _type(type),
            _byteSize(byteSize),
            _slot(slot) {}
        ~CGPUBufferResource() override = default;

        static CGPUBufferResource Invalid()
        {
            return CGPUBufferResource(
                0, 0, 0, EGPUBufferType::Unknown, 0, 0
            );
        }

        inline types::qword GetDeviceMemory() const { return _deviceMemory; }

        inline EGPUBufferType GetBufferType() const { return _type; }

        inline types::usize GetByteSize() const { return _byteSize; }

        inline types::s32 GetSlot() const { return _slot; }
    };
}