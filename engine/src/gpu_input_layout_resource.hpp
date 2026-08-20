// gpu_input_layout.hpp

#pragma once

#include "gpu_resource.hpp"
#include "types.hpp"

namespace triton
{
    class CGPUInputLayoutResource : public cGPUResource
    {
    public:
        explicit CGPUInputLayoutResource() = default;
        explicit CGPUInputLayoutResource(
            types::qword instance,
            types::qword viewInstance
        ) : cGPUResource(instance, viewInstance) {}
        ~CGPUInputLayoutResource() override = default;
    };
}