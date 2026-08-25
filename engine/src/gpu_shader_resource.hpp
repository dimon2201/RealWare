// gpu_shader.hpp

#pragma once

#include <vector>
#include "gpu_resource.hpp"
#include "types.hpp"

namespace triton
{
    class CGPUShaderResource : public CGPUResource
    {
        types::dword _stageMask = 0;
        std::vector<types::qword> _shaderModules;

    public:
        explicit CGPUShaderResource(
            types::qword instance,
            types::qword viewInstance,
            types::dword stageMask,
            const std::vector<types::qword> shaderModules
        ) : CGPUResource(instance, viewInstance), _stageMask(stageMask), _shaderModules(shaderModules) {}
        ~CGPUShaderResource() override = default;

        static CGPUShaderResource Invalid()
        {
            return CGPUShaderResource(
                0, 0, 0, std::vector<types::qword>()
            );
        }

        inline types::dword GetStageMask() const { return _stageMask; }

        inline const std::vector<types::qword>& GetModules() const { return _shaderModules; }
    };
}