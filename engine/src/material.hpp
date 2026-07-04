// material.hpp

#pragma once

#include "math.hpp"
#include "texture_subsystem.hpp"

namespace triton
{
    struct SMaterial
    {
        HTexture diffuseTexture;
        HTexture normalTexture;
        HTexture roughnessTexture;
        HTexture metallicTexture;
        cVector4 diffuseColor = cVector4(0.0f);
    };
}