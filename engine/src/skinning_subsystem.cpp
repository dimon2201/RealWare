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

triton::SSkinData triton::XSkinningSubsystem::CreateSkin(const SEvaluatedFrame& frame)
{
    const SSkeleton& skeleton = _context->GetSubsystem<XSkeletonSubsystem>()->Get(frame.skeleton);
    const usize boneCount = skeleton.bones.size();

    SSkinData sd = {};

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

    sd.globSkinnedBoneOffset = GetBufferSize();

    for (usize i = 0; i < boneCount; ++i)
    {
        sbdArr[i] = Create();
        SSkinnedBoneData& sbd = Get(sbdArr[i]);
        sbd.modelMatrix = tempMatrixBuffer[i] * skeleton.bones[i].modelSpaceToThisBoneSpace;

        // Sync with GPU
        SGPUSkinnedBoneLayout gpusbl;
        gpusbl.modelMatrix = sbd.modelMatrix;
        _uploader->WriteField<SGPUSkinnedBoneLayout>(
            sbdArr[i],
            0,
            gpusbl
        );
    }

    sd.skinnedBones = sbdArr;

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
    _uploader = _context->Create<XUploader<XSkinningSubsystem, HSkinnedBone, SGPUSkinnedBoneLayout>>(
        _context,
        _skinnedBoneBuffer,
        caps->maxSkinnedBoneCount,
        K_TRUE
    );
}

void triton::XSkinningSubsystem::Free()
{
    _context->Destroy<XUploader<XSkinningSubsystem, HSkinnedBone, SGPUSkinnedBoneLayout>>(_uploader);
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