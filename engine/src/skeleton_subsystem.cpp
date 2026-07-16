// skeleton_subsystem.cpp

#include "skeleton_subsystem.hpp"
#include "skeleton.hpp"
#include "uploader.hpp"

using namespace types;

triton::HSkeleton triton::XSkeletonSubsystem::CreateSkeleton(
    const std::vector<SBone>& bones,
    usize globSkinnedBoneOffset,
    const cMatrix4& accumulatedRootTransform
)
{
    HSkeleton skeleton = Create();
    SSkeleton& s = Get(skeleton);
    s.accumulatedRootTransform = accumulatedRootTransform;
    s.bones = bones;
    s.globSkinnedBoneOffset = globSkinnedBoneOffset;

    _uploader->WriteFieldToStaging<decltype(s.globSkinnedBoneOffset)>(
        skeleton,
        0,
        s.globSkinnedBoneOffset
    );

    return skeleton;
}

void triton::XSkeletonSubsystem::DestroySkeleton(const HSkeleton& skeleton)
{
    Destroy(skeleton);
}

void triton::XSkeletonSubsystem::SetSkin(const HSkeleton& skeleton, const SSkinData& skin)
{
    SSkeleton& s = Get(skeleton);
    s.globSkinnedBoneOffset = skin.globSkinnedBoneOffset;

    _uploader->WriteFieldToStaging<decltype(s.globSkinnedBoneOffset)>(
        skeleton,
        0,
        s.globSkinnedBoneOffset
    );
}

void triton::XSkeletonSubsystem::Init()
{
    XRenderSubsystem* renderSubsystem = _context->GetSubsystem<XRenderSubsystem>();
    IApplication* app = _context->GetSubsystem<cEngine>()->GetApplication();
    const sCapabilities* caps = app->GetCapabilities();
    renderSubsystem->PushCommand(SRenderCommand(
        ERenderCommand::CREATE_BUFFER,
        (cpuword)cBuffer::eType::STORAGE,
        (cpuword)nullptr,
        caps->maxSkeletonCount * sizeof(SGPUSkeletonLayout),
        3
    ));
    _skeletonBuffer = renderSubsystem->FetchResult<cBuffer*>();
    _uploader = _context->Create<XUploader<SSkeleton, HSkeleton, XLinearArray<SSkeleton>, SGPUSkeletonLayout>>(
        _context,
        _skeletonBuffer,
        caps->maxSkeletonCount,
        K_FALSE
    );
}

void triton::XSkeletonSubsystem::Free()
{
    _context->Destroy<XUploader<SSkeleton, HSkeleton, XLinearArray<SSkeleton>, SGPUSkeletonLayout>>(_uploader);
    XRenderSubsystem* renderSubsystem = _context->GetSubsystem<XRenderSubsystem>();
    renderSubsystem->PushCommand(SRenderCommand(
        ERenderCommand::DESTROY_BUFFER,
        (cpuword)_skeletonBuffer,
        0,
        0,
        0
    ));
}

void triton::XSkeletonSubsystem::Update()
{
}