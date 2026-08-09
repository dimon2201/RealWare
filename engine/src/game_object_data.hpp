// game_object_data.hpp

#pragma once

#include <string>
#include "batch_instance.hpp"
#include "handle.hpp"
#include "material_data.hpp"
#include "math.hpp"

namespace triton
{
    struct SGameObjectData
    {
        struct THandle : public SHandle {};

        struct TGPULayout {};

        std::string name = {};
        ERenderInstanceMotionType motionType = ERenderInstanceMotionType::Static;
        HStaticRenderInstance staticRenderInstance;
        HDynamicRenderInstance dynamicRenderInstance;
        HMaterial material;
        cVector3 worldPosition = cVector3(0.0f);
        cVector3 worldRotation = cVector3(0.0f);
        cVector3 scale = cVector3(1.0f);
        HSkeleton skeleton = {};
    };
}