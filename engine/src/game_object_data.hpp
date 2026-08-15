// game_object.hpp

#pragma once

#include <string>
#include <optional>
#include "object.hpp"
#include "handle.hpp"
#include "material_data.hpp"
#include "skeleton_data.hpp"
#include "render_instance_data.hpp"
#include "batch_data.hpp"
#include "animation.hpp"
#include "skinning_data.hpp"
#include "model3d_data.hpp"
#include "math.hpp"

namespace triton
{
    struct SGameObjectData
    {
        struct THandle : public SHandle {};

        struct TGPULayout {};

        std::string name = {};
        ERenderInstanceMotionType motionType = ERenderInstanceMotionType::Static;
        SBatchData::THandle batch;
        SStaticRenderInstanceData::THandle staticRenderInstance;
        SDynamicRenderInstanceData::THandle dynamicRenderInstance;
        SSkeletonData::THandle skeleton;
        SSkinData::THandle skin;
        std::vector<SAnimationData::THandle> animations = {};
    };
}