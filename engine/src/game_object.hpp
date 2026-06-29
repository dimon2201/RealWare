// game_object.hpp

#pragma once

#include <string>
#include "batch_storage.hpp"

namespace triton
{
    struct SGameObject
    {
        std::string name = {};
        SBatchInstance renderable;
    };
}