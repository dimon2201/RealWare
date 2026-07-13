// animation_subsystem.cpp

#include "animation_subsystem.hpp"
#include "render_subsystem.hpp"
#include "skeleton_subsystem.hpp"
#include "graphics_resource_backend.hpp"
#include "animation.hpp"
#include "bone.hpp"
#include "handles.hpp"
#include "uploader.hpp"
#include "context.hpp"

using namespace types;

triton::HAnimation triton::XAnimationSubsystem::CreateAnimation(
    const std::string& name,
    types::f32 duration,
    types::f32 ticksPerSecond,
    HSkeleton skeleton,
    const std::vector<SAnimationBone>& bones
)
{
    HAnimation animation = Create();
    SAnimation& a = Get(animation);
    a.name = name;
    a.duration = duration;
    a.ticksPerSecond = ticksPerSecond;
    a.skeleton = skeleton;
    a.animBones = bones;
    
    return animation;
}

void triton::XAnimationSubsystem::DestroyAnimation(const HAnimation& animation)
{
    Destroy(animation);
}

triton::SEvaluatedFrame triton::XAnimationSubsystem::Evaluate(
    const HAnimation& animation,
    f32 time
)
{
    const SAnimation& a = Get(animation);

    SEvaluatedFrame frame = {};
    frame.skeleton = a.skeleton;
    frame.bones.resize(a.animBones.size());

    std::fill(
        frame.bones.begin(),
        frame.bones.end(),
        SEvaluatedBone()
    );

    if (a.duration > 0.0f)
        time = std::fmod(time, a.duration);

    for (const SAnimationBone& animBone : a.animBones)
    {
        cVector3 posePosition = cVector3(0.0f);
        cQuaternion poseRotation = cQuaternion();
        cVector3 poseScale = cVector3(0.0f);

        // TODO: use proper math backend for lerp/slerp and other calculations
        // Rewrite this
        // |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
        // VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV

        if (!animBone.positionKeys.empty())
        {
            if (animBone.positionKeys.size() == 1)
            {
                posePosition = animBone.positionKeys[0].position;
            }
            else
            {
                usize next = 1;
                while (next < animBone.positionKeys.size() &&
                    time >= animBone.positionKeys[next].time)
                {
                    ++next;
                }
                if (next == animBone.positionKeys.size())
                {
                    posePosition = animBone.positionKeys.back().position;
                }
                else
                {
                    const auto& a = animBone.positionKeys[next - 1];
                    const auto& b = animBone.positionKeys[next];
                    f32 factor = (time - a.time) / (b.time - a.time);
                    posePosition = cVector3(glm::mix(a.position._vec, b.position._vec, factor));
                }
            }
        }

        if (!animBone.rotationKeys.empty())
        {
            if (animBone.rotationKeys.size() == 1)
            {
                poseRotation = animBone.rotationKeys[0].rotation;
            }
            else
            {
                usize next = 1;
                while (next < animBone.rotationKeys.size() &&
                    time >= animBone.rotationKeys[next].time)
                {
                    ++next;
                }
                if (next == animBone.rotationKeys.size())
                {
                    poseRotation = animBone.rotationKeys.back().rotation;
                }
                else
                {
                    const auto& a = animBone.rotationKeys[next - 1];
                    const auto& b = animBone.rotationKeys[next];
                    f32 factor = (time - a.time) / (b.time - a.time);
                    poseRotation = cQuaternion(glm::slerp(a.rotation._quat, b.rotation._quat, factor));
                }
            }
        }

        if (!animBone.scaleKeys.empty())
        {
            if (animBone.scaleKeys.size() == 1)
            {
                poseScale = animBone.scaleKeys[0].scale;
            }
            else
            {
                usize next = 1;
                while (next < animBone.scaleKeys.size() &&
                    time >= animBone.scaleKeys[next].time)
                {
                    ++next;
                }
                if (next == animBone.scaleKeys.size())
                {
                    poseScale = animBone.scaleKeys.back().scale;
                }
                else
                {
                    const auto& a = animBone.scaleKeys[next - 1];
                    const auto& b = animBone.scaleKeys[next];
                    f32 factor = (time - a.time) / (b.time - a.time);
                    poseScale = cVector3(glm::mix(a.scale._vec, b.scale._vec, factor));
                }
            }
        }

        glm::mat4 matrix = glm::mat4(1.0f);
        matrix = glm::translate(matrix, posePosition._vec);
        matrix *= glm::mat4_cast(poseRotation._quat);
        matrix = glm::scale(matrix, poseScale._vec);

        SEvaluatedBone eb = {};
        eb.transformMatrix = cMatrix4(matrix);

        frame.bones[animBone.localBoneIndex] = eb;
    }

    return frame;
}