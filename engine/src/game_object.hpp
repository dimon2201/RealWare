// game_object.hpp

#pragma once

#include <string>
#include "batch_instance.hpp"
#include "material.hpp"
#include "math.hpp"

namespace triton
{
    struct SGameObjectData
    {
        SGameObjectData() {}

        std::string name = {};
        ERenderInstanceMotionType motionType = ERenderInstanceMotionType::Static;
        union
        {
            HRenderInstance staticRenderInstance;
            HRenderInstance dynamicRenderInstance;
        };
        HMaterial material;
        cVector3 worldPosition = cVector3(0.0f);
        cVector3 worldRotation = cVector3(0.0f);
        cVector3 scale = cVector3(1.0f);
        HSkeleton skeleton = {};
    };
}