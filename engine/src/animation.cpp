// animation.cpp

#include "animation.hpp"
#include "skeleton_pool.hpp"

using namespace types;

triton::XAnimation::XAnimation(
    cContext* context,
    s32 poolIndex,
    const std::string& name,
    f32 duration,
    f32 ticksPerSecond,
    const std::vector<SAnimationKey> keys
) : iObject(context, poolIndex), _name(name), _duration(duration), _ticksPerSecond(ticksPerSecond), _keys(keys) {}

std::optional<triton::SAnimationFrame> triton::XAnimation::EvaluateFrame(
    const XSkeleton::THandle& skeleton,
    types::f32 time
)
{
    const XSkeleton& sk = *_context->GetPool<CSkeletonPool>()->Get(skeleton);

    const auto& bones = sk.GetBones();
    SAnimationFrame frame = {};
    frame.frameBones.resize(bones.size());
    for (usize i = 0; i < bones.size(); ++i)
        frame.frameBones[i].transformMatrix = bones[i].localMatrix;

    if (_duration > 0.0f)
        time = std::fmod(time, _duration);

    for (const SAnimationKey& animKey : _keys)
    {
        cVector3 posePosition = cVector3(0.0f);
        cQuaternion poseRotation = cQuaternion();
        cVector3 poseScale = cVector3(1.0f);

        // TODO: use proper math backend for lerp/slerp and other calculations
        // Rewrite this
        // |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
        // VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV

        if (!animKey.positionKeys.empty())
        {
            if (animKey.positionKeys.size() == 1)
            {
                posePosition = animKey.positionKeys[0].position;
            }
            else
            {
                usize next = 1;
                while (next < animKey.positionKeys.size() &&
                    time >= animKey.positionKeys[next].time)
                {
                    ++next;
                }
                if (next == animKey.positionKeys.size())
                {
                    posePosition = animKey.positionKeys.back().position;
                }
                else
                {
                    const auto& a = animKey.positionKeys[next - 1];
                    const auto& b = animKey.positionKeys[next];
                    f32 factor = (time - a.time) / (b.time - a.time);
                    posePosition = cVector3(glm::mix(a.position._vec, b.position._vec, factor));
                }
            }
        }

        if (!animKey.rotationKeys.empty())
        {
            if (animKey.rotationKeys.size() == 1)
            {
                poseRotation = animKey.rotationKeys[0].rotation;
            }
            else
            {
                usize next = 1;
                while (next < animKey.rotationKeys.size() &&
                    time >= animKey.rotationKeys[next].time)
                {
                    ++next;
                }
                if (next == animKey.rotationKeys.size())
                {
                    poseRotation = animKey.rotationKeys.back().rotation;
                }
                else
                {
                    const auto& a = animKey.rotationKeys[next - 1];
                    const auto& b = animKey.rotationKeys[next];
                    f32 factor = (time - a.time) / (b.time - a.time);
                    poseRotation = cQuaternion(glm::slerp(a.rotation._quat, b.rotation._quat, factor));
                }
            }
        }

        if (!animKey.scaleKeys.empty())
        {
            if (animKey.scaleKeys.size() == 1)
            {
                poseScale = animKey.scaleKeys[0].scale;
            }
            else
            {
                usize next = 1;
                while (next < animKey.scaleKeys.size() &&
                    time >= animKey.scaleKeys[next].time)
                {
                    ++next;
                }
                if (next == animKey.scaleKeys.size())
                {
                    poseScale = animKey.scaleKeys.back().scale;
                }
                else
                {
                    const auto& a = animKey.scaleKeys[next - 1];
                    const auto& b = animKey.scaleKeys[next];
                    f32 factor = (time - a.time) / (b.time - a.time);
                    poseScale = cVector3(glm::mix(a.scale._vec, b.scale._vec, factor));
                }
            }
        }

        glm::mat4 matrix = glm::mat4(1.0f);
        matrix = glm::translate(matrix, posePosition._vec);
        matrix *= glm::mat4_cast(poseRotation._quat);
        matrix = glm::scale(matrix, poseScale._vec);

        SAnimationFrameBone fb = {};
        fb.transformMatrix = cMatrix4(matrix);

        frame.frameBones[animKey.localBoneIndex] = fb;
    }

    return frame;
}