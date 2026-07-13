// skeleton_subsystem.cpp

#include "skeleton_subsystem.hpp"
#include "skeleton.hpp"
#include "uploader.hpp"

using namespace types;

triton::HSkeleton triton::XSkeletonSubsystem::CreateSkeleton(const std::vector<SBone>& bones)
{
    HSkeleton skeleton = Create();
    SSkeleton& s = Get(skeleton);
    s.globBoneOffset = _totalBoneCount;
    s.bones = bones;
    _totalBoneCount += s.bones.size();

    return skeleton;
}

void triton::XSkeletonSubsystem::DestroySkeleton(const HSkeleton& skeleton)
{
    Destroy(skeleton);
}

void triton::XSkeletonSubsystem::Init()
{
    XRenderSubsystem* renderSubsystem = _context->GetSubsystem<XRenderSubsystem>();
    IApplication* app = _context->GetSubsystem<cEngine>()->GetApplication();
    const sCapabilities* caps = app->GetCapabilities();
    _uploader = _context->Create<XUploader<SSkeletonGPULayout>>(
        _context,
        _skeletonBuffer,
        caps->maxSkeletonCount,
        K_FALSE
    );
}

void triton::XSkeletonSubsystem::Free()
{
    _context->Destroy<XUploader<SSkeletonGPULayout>>(_uploader);
}

void triton::XSkeletonSubsystem::Update()
{
    _uploader->Update();
}