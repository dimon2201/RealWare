// shader_texture_binding.hpp

#pragma once

#include <string>
#include "gpu_texture_resource.hpp"

namespace triton
{
    struct SShaderTextureBinding
    {
        SShaderTextureBinding(
            const std::string& name,
            const CGPUTextureResource& texture
        ) : name(name), texture(texture) {}

        std::string name = "";
        CGPUTextureResource texture;
    };
}