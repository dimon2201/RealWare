// gpu_shader.hpp

#pragma once

#include "gpu_resource.hpp"
#include "types.hpp"

namespace triton
{
    class CGPUShaderResource : public CGPUResource
    {
    public:
        explicit CGPUShaderResource(
            types::qword instance,
            types::qword viewInstance
        ) : CGPUResource(instance, viewInstance) {}
        ~CGPUShaderResource() override = default;

        static CGPUShaderResource Invalid()
        {
            return CGPUShaderResource(
                0, 0
            );
        }
    };
}