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

triton::XSkinningSubsystem::XSkinningSubsystem(cContext* context)
    : ISubsys(context),
      CUploader<SSkinningData, HSkinning, XLinearArray<SSkinningData>, SGPUSkinningLayout>(
        context,
        (cGPUResource**)&_skinningGPUBuffer,
        context->GetSubsystem<cEngine>()->GetCapabilities()->maxSkinnedBoneCount,
        K_FALSE
    )
{
    const sCapabilities* caps = context->GetSubsystem<cEngine>()->GetCapabilities();

    XRenderSubsystem* renderSubsystem = context->GetSubsystem<XRenderSubsystem>();
    renderSubsystem->PushCommand(SRenderCommand(
        ERenderCommand::CREATE_BUFFER,
        (cpuword)cBuffer::eType::STORAGE,
        (cpuword)nullptr,
        caps->maxSkinnedBoneCount * sizeof(SGPUSkeletonLayout),
        4
    ));
    _skinningGPUBuffer = renderSubsystem->FetchResult<cBuffer*>();
}

triton::XSkinningSubsystem::~XSkinningSubsystem()
{
    XRenderSubsystem* renderSubsystem = _context->GetSubsystem<XRenderSubsystem>();
    renderSubsystem->PushCommand(SRenderCommand(
        ERenderCommand::DESTROY_BUFFER,
        (cpuword)_skinningGPUBuffer,
        0,
        0,
        0
    ));
}

triton::SSkinData triton::XSkinningSubsystem::CreateSkin(
    const HSkeleton& skeleton,
    const SFrame& frame
)
{
    const SSkeletonData& skeletonData = _context->GetSubsystem<XSkeletonSubsystem>()->Get(skeleton);
    const usize boneCount = skeletonData.bones.size();

    // Bone transform in Local space
    std::vector<cMatrix4> totalTransform = {};
    totalTransform.resize(boneCount);
    for (usize boneIndex = 0; boneIndex < boneCount; ++boneIndex)
    {
        if (skeletonData.bones[boneIndex].localParentBoneIndex == -1)
            CalculateBone(skeletonData.bones, boneIndex, frame, totalTransform);
    }

    // Record current skinned bone count in CHandleAllocator's array
    SSkinData sd = {};
    sd.globSkinnedBoneBufferOffset = GetSize();

    // Bone transform in Model space
    std::vector<HSkinning> skinnedBones = {};
    skinnedBones.resize(boneCount);
    std::vector<SGPUSkinningLayout> gpuSkinnedBones = {};
    gpuSkinnedBones.resize(boneCount);
    for (usize boneIndex = 0; boneIndex < boneCount; ++boneIndex)
    {
        skinnedBones[boneIndex] = Create(); // Reserve space for skinned bones in CHandleAllocator's array
        SSkinningData& skd = Get(skinnedBones[boneIndex]);
        skd.modelMatrix =
            skeletonData.accumulatedRootTransform *
            totalTransform[boneIndex] *
            skeletonData.bones[boneIndex].modelMatrix;

        gpuSkinnedBones[boneIndex].modelMatrix = skd.modelMatrix;
    }
    sd.skinnedBones = skinnedBones;

    // Sync with GPU
    SGPUSkinningLayout gsl;
    gsl.modelMatrix = gsl.modelMatrix;
    WriteToStaging(
        GetHandleBufferIndex(skinnedBones[0]),
        &gpuSkinnedBones.data()[0],
        boneCount
    );

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
        caps->maxSkinnedBoneCount * sizeof(SGPUSkinningLayout),
        4
    ));
    _skinningGPUBuffer = renderSubsystem->FetchResult<cBuffer*>();
}

void triton::XSkinningSubsystem::Free()
{
    XRenderSubsystem* renderSubsystem = _context->GetSubsystem<XRenderSubsystem>();
    renderSubsystem->PushCommand(SRenderCommand(
        ERenderCommand::DESTROY_BUFFER,
        (cpuword)_skinningGPUBuffer,
        0,
        0,
        0
    ));
}

void triton::XSkinningSubsystem::Update()
{
    UploadStagingToGpuIfDirty(_context->GetSubsystem<XRenderSubsystem>());
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