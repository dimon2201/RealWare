// animation_key.hpp

#pragma once

#include <vector>
#include "math.hpp"
#include "types.hpp"

namespace triton
{
    struct SBonePositionKey
    {
        types::f32 time = 0.0f;
        cVector3 position = cVector3(0.0f);
    };

    struct SBoneRotationKey
    {
        types::f32 time = 0.0f;
        cQuaternion rotation = cQuaternion();
    };

    struct SBoneScaleKey
    {
        types::f32 time = 0.0f;
        cVector3 scale = cVector3(0.0f);
    };

    struct SAnimationKey
    {
        types::usize localBoneIndex = 0;
        std::vector<SBonePositionKey> positionKeys = {};
        std::vector<SBoneRotationKey> rotationKeys = {};
        std::vector<SBoneScaleKey> scaleKeys = {};
    };
}