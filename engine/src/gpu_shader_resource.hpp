// gpu_shader.hpp

#pragma once

#include <vector>
#include "gpu_resource.hpp"
#include "types.hpp"

namespace triton
{
    class CGPUShaderResource : public CGPUResource
    {
        std::vector<types::qword> _shaderModules;

    public:
        explicit CGPUShaderResource(
            types::qword instance,
            types::qword viewInstance,
            const std::vector<types::qword> shaderModules
        ) : CGPUResource(instance, viewInstance), _shaderModules(shaderModules) {}
        ~CGPUShaderResource() override = default;

        static CGPUShaderResource Invalid()
        {
            return CGPUShaderResource(
                0, 0, std::vector<types::qword>()
            );
        }

        inline const std::vector<types::qword>& GetModules() const { return _shaderModules; }
    };
}