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

std::vector<triton::HSkinnedBone> triton::XSkinningSubsystem::CreateSkin(const SEvaluatedFrame& frame)
{
    const SSkeleton& skeleton = _context->GetSubsystem<XSkeletonSubsystem>()->Get(frame.skeleton);
    const usize boneCount = skeleton.bones.size();

    std::vector<HSkinnedBone> sbdArr = {};
    sbdArr.resize(boneCount);

    std::vector<cMatrix4> tempMatrixBuffer = {};
    tempMatrixBuffer.resize(boneCount);

    for (usize i = 0; i < boneCount; ++i)
    {
        const SBone& bone = skeleton.bones[i];
        if (bone.parentLocalBoneIndex < 0)
        {
            tempMatrixBuffer[i] = frame.bones[i].transformMatrix;
        }
        else
        {
            tempMatrixBuffer[i] =
                tempMatrixBuffer[bone.parentLocalBoneIndex] *
                frame.bones[i].transformMatrix;
        }
    }

    for (usize i = 0; i < boneCount; ++i)
    {
        sbdArr[i] = Create();
        Get(sbdArr[i]).modelMatrix = tempMatrixBuffer[i] * skeleton.bones[i].modelSpaceToThisBoneSpace;
    }

    return sbdArr;
}

void triton::XSkinningSubsystem::DestroySkin(const std::vector<HSkinnedBone>& skin)
{
    for (auto& sbd : skin)
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
    _uploader = _context->Create<XUploader<HSkinnedBone, XSkinningSubsystem, SGPUSkinnedBoneLayout>>(
        _context,
        _skinnedBoneBuffer,
        caps->maxSkinnedBoneCount,
        K_TRUE
    );
}

void triton::XSkinningSubsystem::Free()
{
    _context->Destroy<XUploader<HSkinnedBone, XSkinningSubsystem, SGPUSkinnedBoneLayout>>(_uploader);
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
    _uploader->Update();
}