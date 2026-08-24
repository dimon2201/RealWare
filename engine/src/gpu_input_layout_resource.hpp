// gpu_input_layout.hpp

#pragma once

#include "gpu_resource.hpp"
#include "types.hpp"

namespace triton
{
    class CGPUInputLayoutResource : public CGPUResource
    {
    public:
        explicit CGPUInputLayoutResource(
            types::qword instance,
            types::qword viewInstance
        ) : CGPUResource(instance, viewInstance) {}
        ~CGPUInputLayoutResource() override = default;

        static CGPUInputLayoutResource Invalid()
        {
            return CGPUInputLayoutResource(
                0, 0
            );
        }
    };
}