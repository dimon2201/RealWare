// gpu_render_pass_resource.hpp

#pragma once

#include "gpu_resource.hpp"
#include "types.hpp"

namespace triton
{
    class CGPURenderPassResource : public cGPUResource
    {
    public:
        explicit CGPURenderPassResource() = default;
        explicit CGPURenderPassResource(
            types::qword instance,
            types::qword viewInstance
        ) : cGPUResource(instance, viewInstance) {}
        ~CGPURenderPassResource() override = default;
    };
}