// game_object.hpp

#pragma once

#include <string>
#include "batch_instance.hpp"
#include "material.hpp"
#include "math.hpp"

namespace triton
{
    struct SGameObject
    {
        std::string name = {};
        SBatchInstance renderable;
        HMaterial material;
        cVector3 worldPosition = cVector3(0.0f);
        cVector3 worldRotation = cVector3(0.0f);
    };
}