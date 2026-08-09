// skinning_subsystem.cpp

#include "skinning_subsystem.hpp"
#include "skeleton_subsystem.hpp"
#include "graphics_resource_backend.hpp"
#include "animation.hpp"
#include "bone.hpp"
#include "handles.hpp"
#include "uploader.hpp"
#include "context.hpp"
#include "skinned_bones_pool.hpp"
#include "skin_pool.hpp"
#include "skeleton_pool.hpp"

using namespace types;

triton::XSkinningSubsystem::XSkinningSubsystem(cContext* context) : ISubsys(context)
{
    _skinnedBonesPool = CObjectAllocator::Create<XSkinnedBonesPool>(
        64,
        _context,
        K_TRUE,
        (s32)4,
        cBuffer::eType::STORAGE
    );
    _skinPool = CObjectAllocator::Create<XSkinPool>(
        64,
        _context,
        K_TRUE
    );
}

triton::XSkinningSubsystem::~XSkinningSubsystem()
{
    CObjectAllocator::Destroy<XSkinPool>(_skinPool);
    CObjectAllocator::Destroy<XSkinnedBonesPool>(_skinnedBonesPool);
}

std::optional<triton::SSkinData::THandle> triton::XSkinningSubsystem::Create(
    const SSkeletonData::THandle& skeleton,
    const SFrame& frame
)
{
    auto skelDataResult = _context->GetSubsystem<XSkeletonSubsystem>()->GetPool()->Get(skeleton);
    if (!skelDataResult.has_value())
        return std::nullopt;
    auto skelData = *skelDataResult;
        
    const usize boneCount = skelData.get().bones.size();

    // Bone transform in Local space
    std::vector<cMatrix4> totalTransform = {};
    totalTransform.resize(boneCount);
    for (usize boneIndex = 0; boneIndex < boneCount; ++boneIndex)
    {
        if (skelData.get().bones[boneIndex].localParentBoneIndex == -1)
            CalculateBone(skelData.get().bones, boneIndex, frame, totalTransform);
    }

    // Record current skinned bone count
    usize globSkinnedBoneBufferOffset = _skinnedBonesPool->GetSize();

    // Calculate bone transform in Model space
    std::vector<SSkinnedBoneData::THandle> skinnedBones;
    skinnedBones.resize(boneCount);
    for (usize boneIndex = 0; boneIndex < boneCount; ++boneIndex)
    {
        skinnedBones[boneIndex] = *_skinnedBonesPool->Create(); // Reserve space for skinned bones
        SSkinnedBoneData& skbd = *_skinnedBonesPool->Get(skinnedBones[boneIndex]);
        skbd.modelMatrix =
            skelData.get().accumulatedRootTransform *
            totalTransform[boneIndex] *
            skelData.get().bones[boneIndex].modelMatrix;
        _skinnedBonesPool->WriteToStaging(
            globSkinnedBoneBufferOffset + boneIndex,
            skbd
        );
    }

    SSkinData::THandle skin = *_skinPool->Create();
    SSkinData& sd = *_skinPool->Get(skin);
    sd.skinnedBones = skinnedBones;

    return skin;
}

void triton::XSkinningSubsystem::Destroy(const SSkinData::THandle& skin)
{
    SSkinData& sd = *_skinPool->Get(skin);
    for (auto& sbd : sd.skinnedBones)
        _skinnedBonesPool->Destroy(sbd);
    _skinPool->Destroy(skin);
}

void triton::XSkinningSubsystem::Update()
{
    _skinnedBonesPool->Update();
    _skinPool->Update();
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