// skeleton_subsystem.cpp

#include "skeleton_subsystem.hpp"
#include "skeleton.hpp"
#include "uploader.hpp"

using namespace types;

triton::XSkeletonSubsystem::XSkeletonSubsystem(cContext* context)
    : ISubsys(context),
      CUploader<SSkeletonData, HSkeleton, XLinearArray<SSkeletonData>, SGPUSkeletonLayout>(
        context,
        (cGPUResource**)&_skeletonGPUBuffer,
        context->GetSubsystem<cEngine>()->GetCapabilities()->maxSkeletonCount,
        K_FALSE
    )
{
    const sCapabilities* caps = context->GetSubsystem<cEngine>()->GetCapabilities();

    XRenderSubsystem* renderSubsystem = context->GetSubsystem<XRenderSubsystem>();
    renderSubsystem->PushCommand(SRenderCommand(
        ERenderCommand::CREATE_BUFFER,
        (cpuword)cBuffer::eType::STORAGE,
        (cpuword)nullptr,
        caps->maxSkeletonCount * sizeof(SGPUSkeletonLayout),
        3
    ));
    _skeletonGPUBuffer = renderSubsystem->FetchResult<cBuffer*>();
}

triton::XSkeletonSubsystem::~XSkeletonSubsystem()
{
    XRenderSubsystem* renderSubsystem = _context->GetSubsystem<XRenderSubsystem>();
    renderSubsystem->PushCommand(SRenderCommand(
        ERenderCommand::DESTROY_BUFFER,
        (cpuword)_skeletonGPUBuffer,
        0,
        0,
        0
    ));
}

triton::HSkeleton triton::XSkeletonSubsystem::CreateSkeleton(
    const std::vector<SBone>& bones,
    const cMatrix4& accumulatedRootTransform
)
{
    HSkeleton skeleton = CHandleAllocator::Create();
    SSkeletonData& s = CHandleAllocator::Get(skeleton);
    s.accumulatedRootTransform = accumulatedRootTransform;
    s.bones = bones;

    return skeleton;
}

void triton::XSkeletonSubsystem::DestroySkeleton(const HSkeleton& skeleton)
{
    CHandleAllocator::Destroy(skeleton);
}

void triton::XSkeletonSubsystem::Init()
{
}

void triton::XSkeletonSubsystem::Free()
{
}

void triton::XSkeletonSubsystem::Update()
{
    UploadStagingToGpuIfDirty(_context->GetSubsystem<XRenderSubsystem>());
}

void triton::XSkeletonSubsystem::SetSkin(const HSkeleton& skeleton, const SSkinData& skin)
{
    SSkeletonData& s = CHandleAllocator::Get(skeleton);
    s.globSkinnedBoneBufferOffset = skin.globSkinnedBoneBufferOffset;

    SGPUSkeletonLayout gsl = ConvertToGPULayout(skeleton);
    WriteToStaging(
        GetHandleBufferIndex(skeleton),
        &gsl,
        1
    );
}

triton::SGPUSkeletonLayout triton::XSkeletonSubsystem::ConvertToGPULayout(const HSkeleton& skeleton)
{
    SSkeletonData& s = CHandleAllocator::Get(skeleton);

    SGPUSkeletonLayout gsl;
    gsl.globSkinnedBoneBufferOffset = s.globSkinnedBoneBufferOffset;

    return gsl;
}