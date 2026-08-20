// skin.cpp

#include "skin.hpp"
#include "context.hpp"
#include "skeleton_pool.hpp"
#include "skinned_bones_pool.hpp"

using namespace types;

triton::XSkin::XSkin(
	cContext* context,
	types::s32 poolIndex,
	const XSkeleton::THandle& skeleton
) : iObject(context, poolIndex)
{
    XSkeleton& sd = *_context->GetPool<CSkeletonPool>()->Get(skeleton);

    CSkinnedBonesPool* skinnedBonesPool = _context->GetPool<CSkinnedBonesPool>();

    std::vector<CSkinnedBone::THandle> skinnedBones = {};
    for (usize i = 0; i < sd.GetBones().size(); i++)
    {
        auto handle = skinnedBonesPool->Create();
        if (handle)
            skinnedBones.push_back(*handle);
    }

    const usize globSkinnedBoneBufferOffset = skinnedBonesPool->GetPackedIndex(skinnedBones[0]);

    _skeleton = skeleton;
    _globSkinnedBoneBufferOffset = globSkinnedBoneBufferOffset;
    _skinnedBones = skinnedBones;
}

triton::XSkin::~XSkin()
{
    for (auto& sbd : _skinnedBones)
        _context->GetPool<CSkinnedBonesPool>()->Destroy(sbd);
}

void triton::XSkin::Skin(const SAnimationFrame& frame)
{
    XSkeleton& skeleton = *_context->GetPool<CSkeletonPool>()->Get(_skeleton);

    const auto& skeletonBones = skeleton.GetBones();
    const usize skeletonBoneCount = skeletonBones.size();

    // Bone transform in Local space
    std::vector<cMatrix4> totalTransform = {};
    totalTransform.resize(skeletonBoneCount);
    for (usize boneIndex = 0; boneIndex < skeletonBoneCount; ++boneIndex)
    {
        if (skeletonBones[boneIndex].localParentBoneIndex == -1)
            CalculateBone(skeletonBones, boneIndex, frame, totalTransform);
    }

    // Calculate bone transform in Model space
    CSkinnedBonesPool* skinnedBonesPool = _context->GetPool<CSkinnedBonesPool>();
    for (usize boneIndex = 0; boneIndex < skeletonBoneCount; ++boneIndex)
    {
        CSkinnedBone& skb = *skinnedBonesPool->Get(_skinnedBones[boneIndex]);
        skb.SetModelMatrix(
            skeleton.GetAccumulatedRootTransform() *
            totalTransform[boneIndex] *
            skeletonBones[boneIndex].modelMatrix);
        skinnedBonesPool->WriteToStaging(
            _globSkinnedBoneBufferOffset + boneIndex,
            skb
        );
    }
}

void triton::XSkin::CalculateBone(
    const std::vector<SSkeletonBone>& bones,
    types::usize boneIndex,
    const SAnimationFrame& frame,
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