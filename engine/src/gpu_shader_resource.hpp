// gpu_shader.hpp

#pragma once

#include "gpu_resource.hpp"
#include "types.hpp"

namespace triton
{
    class CGPUShaderResource : public cGPUResource
    {
    public:
        explicit CGPUShaderResource() = default;
        explicit CGPUShaderResource(
            types::qword instance,
            types::qword viewInstance
        ) : cGPUResource(instance, viewInstance) {}
        ~CGPUShaderResource() override = default;
    };
}