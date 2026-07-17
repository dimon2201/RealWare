// skinning_subsystem.cpp

#include "skinning_subsystem.hpp"
#include "render_subsystem.hpp"
#include "skeleton_subsystem.hpp"
#include "graphics_resource_backend.hpp"
#include "animation.hpp"
#include "bone.hpp"
#include "handles.hpp"
#include "uploader.hpp"
#include "context.hpp"

using namespace types;

triton::SSkinData triton::XSkinningSubsystem::CreateSkin(
    const HSkeleton& skeleton,
    const SFrame& frame
)
{
    const SSkeleton& skeletonData = _context->GetSubsystem<XSkeletonSubsystem>()->Get(skeleton);
    const usize boneCount = skeletonData.bones.size();

    SSkinData sd = {};
    sd.globSkinnedBoneOffset = GetBufferSize();

    std::vector<HSkinnedBone> skinnedBones = {};
    skinnedBones.resize(boneCount);

    std::vector<cMatrix4> totalTransform = {};
    totalTransform.resize(boneCount);

    // Bone transform in Local space
    for (usize boneIndex = 0; boneIndex < boneCount; ++boneIndex)
    {
        if (skeletonData.bones[boneIndex].localParentBoneIndex == -1)
            CalculateBone(skeletonData.bones, boneIndex, frame, totalTransform);
    }

    // Bone transform in Model space
    for (usize boneIndex = 0; boneIndex < boneCount; ++boneIndex)
    {
        skinnedBones[boneIndex] = Create();

        SSkinnedBoneData& sbd = Get(skinnedBones[boneIndex]);
        sbd.modelMatrix =
            skeletonData.accumulatedRootTransform *
            totalTransform[boneIndex] *
            skeletonData.bones[boneIndex].modelMatrix;

        //sbd.modelMatrix = cMatrix4(glm::mat4(1.0f));

        // Sync with GPU
        SGPUSkinnedBoneLayout gpusbl;
        gpusbl.modelMatrix = sbd.modelMatrix;
        _uploader->WriteFieldToStaging<SGPUSkinnedBoneLayout>(
            skinnedBones[boneIndex],
            0,
            gpusbl
        );
    }

    sd.skinnedBones = skinnedBones;

    return sd;
}

void triton::XSkinningSubsystem::DestroySkin(const SSkinData& skin)
{
    for (auto& sbd : skin.skinnedBones)
        Destroy(sbd);
}

void triton::XSkinningSubsystem::Init()
{
    XRenderSubsystem* renderSubsystem = _context->GetSubsystem<XRenderSubsystem>();
    IApplication* app = _context->GetSubsystem<cEngine>()->GetApplication();
    const sCapabilities* caps = app->GetCapabilities();
    renderSubsystem->PushCommand(SRenderCommand(
        ERenderCommand::CREATE_BUFFER,
        (cpuword)cBuffer::eType::STORAGE,
        (cpuword)nullptr,
        caps->maxSkinnedBoneCount * sizeof(SGPUSkinnedBoneLayout),
        4
    ));
    _skinnedBoneBuffer = renderSubsystem->FetchResult<cBuffer*>();
    _uploader = _context->Create<CUploader<SSkinnedBoneData, HSkinnedBone, XLinearArray<SSkinnedBoneData>, SGPUSkinnedBoneLayout>>(
        _context,
        (cGPUResource**)&_skinnedBoneBuffer,
        caps->maxSkinnedBoneCount,
        K_TRUE
    );
}

void triton::XSkinningSubsystem::Free()
{
    _context->Destroy<CUploader<SSkinnedBoneData, HSkinnedBone, XLinearArray<SSkinnedBoneData>, SGPUSkinnedBoneLayout>>(_uploader);
    XRenderSubsystem* renderSubsystem = _context->GetSubsystem<XRenderSubsystem>();
    renderSubsystem->PushCommand(SRenderCommand(
        ERenderCommand::DESTROY_BUFFER,
        (cpuword)_skinnedBoneBuffer,
        0,
        0,
        0
    ));
}

void triton::XSkinningSubsystem::Update()
{
}

void triton::XSkinningSubsystem::CalculateBone(
    const std::vector<SBone>& bones,
    usize boneIndex,
    const SFrame& frame,
    std::vector<cMatrix4>& totalTransform
)
{
    if (bones[boneIndex].localParentBoneIndex == -1)
        totalTransform[boneIndex] = frame.frameBones[boneIndex].transformMatrix;
    else
        totalTransform[boneIndex] =
            totalTransform[bones[boneIndex].localParentBoneIndex] *
            frame.frameBones[boneIndex].transformMatrix;
    for (usize i : bones[boneIndex].localChildBoneIndices)
        CalculateBone(bones, i, frame, totalTransform);
}