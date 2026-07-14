// animation.hpp

#pragma once

namespace triton
{
    class cMatrix4;

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

    struct SAnimation
    {
        std::string name = "";
        types::f32 duration = 0.0f;
        types::f32 ticksPerSecond = 0.0f;
        std::vector<SAnimationKey> animKeys = {};
    };

    struct SFrameBone
    {
        cMatrix4 transformMatrix = cMatrix4();
    };

    struct SFrame
    {
        std::vector<SFrameBone> frameBones = {};
    };
}