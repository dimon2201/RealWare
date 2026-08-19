// gpu_shader.hpp

#pragma once

#include "gpu_resource.hpp"
#include "types.hpp"

namespace triton
{
    class cContext;

    class CGPUShaderResource : public cGPUResource
    {
        TRITON_OBJECT(CGPUShaderResource)

    public:
        explicit CGPUShaderResource() = default;
        explicit CGPUShaderResource(
            cContext* context,
            types::qword instance,
            types::qword viewInstance
        ) : cGPUResource(context, instance, viewInstance) {
        }
        ~CGPUShaderResource() override = default;
    };
}