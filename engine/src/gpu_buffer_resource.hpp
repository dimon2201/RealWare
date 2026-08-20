// gpu_buffer_resource.hpp

#pragma once

#include "gpu_resource.hpp"
#include "gpu_buffer_types.hpp"
#include "types.hpp"

namespace triton
{
    class CGPUBufferResource : public cGPUResource
    {
        EGPUBufferType _type = EGPUBufferType::Unknown;
        types::usize _byteSize = 0;
        types::s32 _slot = -1;

    public:
        explicit CGPUBufferResource() = default;
        explicit CGPUBufferResource(
            types::qword instance,
            types::qword viewInstance,
            EGPUBufferType type,
            types::usize byteSize,
            types::s32 slot
        ) : cGPUResource(instance, viewInstance), _type(type), _byteSize(byteSize), _slot(slot) {}
        ~CGPUBufferResource() override = default;

        inline EGPUBufferType GetBufferType() const { return _type; }

        inline types::usize GetByteSize() const { return _byteSize; }

        inline types::s32 GetSlot() const { return _slot; }
    };
}