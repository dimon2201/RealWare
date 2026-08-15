// skinning_subsystem.cpp

#include "skinning_subsystem.hpp"
#include "skeleton_subsystem.hpp"
#include "graphics_resource_backend.hpp"
#include "animation.hpp"
#include "bone.hpp"
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
        (s32)3,
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

std::optional<triton::SSkinData::THandle> triton::XSkinningSubsystem::Create(const SSkeletonData::THandle& skeleton)
{
    const SSkeletonData& skd = *_context->GetSubsystem<XSkeletonSubsystem>()->GetPool()->Get(skeleton);
    
    std::vector<SSkinnedBoneData::THandle> skinnedBones = {};
    for (usize i = 0; i < skd.bones.size(); i++)
        skinnedBones.push_back(*_skinnedBonesPool->Create());

    const usize globSkinnedBoneBufferOffset = _skinnedBonesPool->GetPackedIndex(skinnedBones[0]);

    SSkinData::THandle skin = *_skinPool->Create();
    SSkinData& sd = *_skinPool->Get(skin);
    sd.skeleton = skeleton;
    sd.globSkinnedBoneBufferOffset = globSkinnedBoneBufferOffset;
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

void triton::XSkinningSubsystem::Skin(
    const SSkinData::THandle& skin,
    const SFrame& frame
)
{
    auto skinDataResult = _skinPool->Get(skin);
    if (!skinDataResult.has_value())
        return;
    const SSkinData& skinData = *skinDataResult;

    const SSkeletonData& skeletonData = *_context->GetSubsystem<XSkeletonSubsystem>()->GetPool()->Get(skinData.skeleton);

    const usize boneCount = skinData.skinnedBones.size();

    // Bone transform in Local space
    std::vector<cMatrix4> totalTransform = {};
    totalTransform.resize(boneCount);
    for (usize boneIndex = 0; boneIndex < boneCount; ++boneIndex)
    {
        if (skeletonData.bones[boneIndex].localParentBoneIndex == -1)
            CalculateBone(skeletonData.bones, boneIndex, frame, totalTransform);
    }

    // Calculate bone transform in Model space
    for (usize boneIndex = 0; boneIndex < boneCount; ++boneIndex)
    {
        SSkinnedBoneData& skbd = *_skinnedBonesPool->Get(skinData.skinnedBones[boneIndex]);
        skbd.modelMatrix =
            skeletonData.accumulatedRootTransform *
            totalTransform[boneIndex] *
            skeletonData.bones[boneIndex].modelMatrix;
        _skinnedBonesPool->WriteToStaging(
            skinData.globSkinnedBoneBufferOffset + boneIndex,
            skbd
        );
    }
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