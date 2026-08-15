// game_object_data.hpp

#pragma once

#include <string>
#include "handle.hpp"
#include "material_data.hpp"
#include "skeleton_data.hpp"
#include "render_instance_data.hpp"
#include "math.hpp"

namespace triton
{
    struct SGameObjectData
    {
        struct THandle : public SHandle {};

        struct TGPULayout {};

        std::string name = {};
        ERenderInstanceMotionType motionType = ERenderInstanceMotionType::Static;
        SStaticRenderInstanceData::THandle staticRenderInstance;
        SDynamicRenderInstanceData::THandle dynamicRenderInstance;
        SMaterialData::THandle material;
        cVector3 worldPosition = cVector3(0.0f);
        cVector3 worldRotation = cVector3(0.0f);
        cVector3 scale = cVector3(1.0f);
        SSkeletonData::THandle skeleton;
    };
}