// game_object.hpp

#pragma once

#include <string>
#include "batch_instance.hpp"
#include "material.hpp"

namespace triton
{
    struct SGameObject
    {
        std::string name = {};
        SBatchInstance renderable;
        HMaterial material;
    };
}