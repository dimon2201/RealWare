// gpu_render_pass_resource.hpp

#pragma once

#include "gpu_resource.hpp"
#include "types.hpp"

namespace triton
{
    class cContext;

    class CGPURenderPassResource : public cGPUResource
    {
        TRITON_OBJECT(CGPURenderPassResource)

    public:
        explicit CGPURenderPassResource() = default;
        explicit CGPURenderPassResource(
            cContext* context,
            types::qword instance,
            types::qword viewInstance
        ) : cGPUResource(context, instance, viewInstance) {}
        ~CGPURenderPassResource() override = default;
    };
}