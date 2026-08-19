// gpu_input_layout.hpp

#pragma once

#include "gpu_resource.hpp"
#include "types.hpp"

namespace triton
{
    class cContext;

    class CGPUInputLayoutResource : public cGPUResource
    {
        TRITON_OBJECT(CGPUInputLayoutResource)

    public:
        explicit CGPUInputLayoutResource() = default;
        explicit CGPUInputLayoutResource(
            cContext* context,
            types::qword instance,
            types::qword viewInstance
        ) : cGPUResource(context, instance, viewInstance) {}
        ~CGPUInputLayoutResource() override = default;
    };
}