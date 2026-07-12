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
    const std::vector<SBoneAnimation>& bones
)
{
    HAnimation animation = Create();
    SAnimation& a = Get(animation);
    a.name = name;
    a.duration = duration;
    a.ticksPerSecond = ticksPerSecond;
    a.skeleton = skeleton;
    a.bones = bones;
    
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
    frame.bones.resize(a.bones.size());

    std::fill(
        frame.bones.begin(),
        frame.bones.end(),
        SEvaluatedBone()
    );

    if (a.duration > 0.0f)
        time = std::fmod(time, a.duration);

    for (const SBoneAnimation& boneAnimation : a.bones)
    {
        cVector3 posePosition = cVector3(0.0f);
        cQuaternion poseRotation = cQuaternion();
        cVector3 poseScale = cVector3(0.0f);

        // TODO: use proper math backend for lerp/slerp and other calculations
        // Rewrite this
        // |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
        // VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV

        if (!boneAnimation.positionKeys.empty())
        {
            if (boneAnimation.positionKeys.size() == 1)
            {
                posePosition = boneAnimation.positionKeys[0].position;
            }
            else
            {
                usize next = 1;
                while (next < boneAnimation.positionKeys.size() &&
                    time >= boneAnimation.positionKeys[next].time)
                {
                    ++next;
                }
                if (next == boneAnimation.positionKeys.size())
                {
                    posePosition = boneAnimation.positionKeys.back().position;
                }
                else
                {
                    const auto& a = boneAnimation.positionKeys[next - 1];
                    const auto& b = boneAnimation.positionKeys[next];
                    f32 factor = (time - a.time) / (b.time - a.time);
                    posePosition = cVector3(glm::mix(a.position._vec, b.position._vec, factor));
                }
            }
        }

        if (!boneAnimation.rotationKeys.empty())
        {
            if (boneAnimation.rotationKeys.size() == 1)
            {
                poseRotation = boneAnimation.rotationKeys[0].rotation;
            }
            else
            {
                usize next = 1;
                while (next < boneAnimation.rotationKeys.size() &&
                    time >= boneAnimation.rotationKeys[next].time)
                {
                    ++next;
                }
                if (next == boneAnimation.rotationKeys.size())
                {
                    poseRotation = boneAnimation.rotationKeys.back().rotation;
                }
                else
                {
                    const auto& a = boneAnimation.rotationKeys[next - 1];
                    const auto& b = boneAnimation.rotationKeys[next];
                    f32 factor = (time - a.time) / (b.time - a.time);
                    poseRotation = cQuaternion(glm::slerp(a.rotation._quat, b.rotation._quat, factor));
                }
            }
        }

        if (!boneAnimation.scaleKeys.empty())
        {
            if (boneAnimation.scaleKeys.size() == 1)
            {
                poseScale = boneAnimation.scaleKeys[0].scale;
            }
            else
            {
                usize next = 1;
                while (next < boneAnimation.scaleKeys.size() &&
                    time >= boneAnimation.scaleKeys[next].time)
                {
                    ++next;
                }
                if (next == boneAnimation.scaleKeys.size())
                {
                    poseScale = boneAnimation.scaleKeys.back().scale;
                }
                else
                {
                    const auto& a = boneAnimation.scaleKeys[next - 1];
                    const auto& b = boneAnimation.scaleKeys[next];
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

        frame.bones[boneAnimation.localBoneIndex] = eb;
    }

    return frame;
}

triton::SSkinMatrices triton::XAnimationSubsystem::Skin(
    const SEvaluatedFrame& frame
)
{
    const SSkeleton& skeleton = _context->GetSubsystem<XSkeletonSubsystem>()->Get(frame.skeleton);
    const usize boneCount = skeleton.bones.size();

    SSkinMatrices skin;
    skin.matrices.resize(boneCount);

    std::vector<cMatrix4> globalMatrices;
    globalMatrices.resize(boneCount);

    for (usize i = 0; i < boneCount; ++i)
    {
        const SBone& bone = skeleton.bones[i];
        if (bone.parentLocalBoneIndex < 0)
        {
            globalMatrices[i] = frame.bones[i].transformMatrix;
        }
        else
        {
            globalMatrices[i] = 
                globalMatrices[bone.parentLocalBoneIndex] *
                frame.bones[i].transformMatrix;
        }
    }

    for (usize i = 0; i < boneCount; ++i)
    {
        skin.matrices[i] =
            globalMatrices[i] *
            skeleton.bones[i].modelSpaceToThisBoneSpace;
    }

    return skin;
}

void triton::XAnimationSubsystem::Init()
{
    XRenderSubsystem* renderSubsystem = _context->GetSubsystem<XRenderSubsystem>();
    IApplication* app = _context->GetSubsystem<cEngine>()->GetApplication();
    const sCapabilities* caps = app->GetCapabilities();
    renderSubsystem->PushCommand(SRenderCommand(
        ERenderCommand::CREATE_BUFFER,
        (cpuword)cBuffer::eType::STORAGE,
        (cpuword)nullptr,
        caps->boneBufferSize,
        4
    ));
    _skinnedBoneBuffer = renderSubsystem->FetchResult<cBuffer*>();
    _uploader = _context->Create<XUploader<SSkinnedBoneGPULayout>>(_context, _skinnedBoneBuffer, caps->maxSkinnedBoneCount);
}

void triton::XAnimationSubsystem::Free()
{
    _context->Destroy<XUploader<SSkinnedBoneGPULayout>>(_uploader);
    XRenderSubsystem* renderSubsystem = _context->GetSubsystem<XRenderSubsystem>();
    renderSubsystem->PushCommand(SRenderCommand(
        ERenderCommand::DESTROY_BUFFER,
        (cpuword)_skinnedBoneBuffer,
        0,
        0,
        0
    ));
}

void triton::XAnimationSubsystem::Update()
{
    _uploader->Update();
}