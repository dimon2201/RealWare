// skeleton_subsystem.cpp

#include "skeleton_subsystem.hpp"
#include "skeleton_pool.hpp"

using namespace types;

triton::XSkeletonSubsystem::XSkeletonSubsystem(cContext* context) : ISubsys(context)
{
    _pool = CObjectAllocator::Create<XSkeletonPool>(
        64,
        _context,
        K_TRUE,
        (s32)3,
        cBuffer::eType::STORAGE
    );
}

triton::XSkeletonSubsystem::~XSkeletonSubsystem()
{
    CObjectAllocator::Destroy<XSkeletonPool>(_pool);
}

std::optional<triton::SSkeletonData::THandle> triton::XSkeletonSubsystem::Create(
    const std::vector<SBone>& bones,
    const cMatrix4& accumulatedRootTransform
)
{
    auto handleResult = _pool->Create();
    if (!handleResult.has_value())
        return std::nullopt;
    auto handle = *handleResult;

    auto valueResult = _pool->Get(handle);
    if (!valueResult.has_value())
        return std::nullopt;
    auto value = *valueResult;

    value.get().accumulatedRootTransform = accumulatedRootTransform;
    value.get().bones = bones;

    return handle;
}

void triton::XSkeletonSubsystem::Destroy(const SSkeletonData::THandle& skeleton)
{
    _pool->Destroy(skeleton);
}

void triton::XSkeletonSubsystem::Init()
{
}

void triton::XSkeletonSubsystem::Free()
{
}

void triton::XSkeletonSubsystem::Update()
{
    _pool->Update();
}

void triton::XSkeletonSubsystem::SetSkin(const SSkeletonData::THandle& skeleton, const SSkinData& skin)
{
    auto valueResult = _pool->Get(skeleton);
    if (!valueResult.has_value())
        return;
    auto value = *valueResult;

    value.get().globSkinnedBoneBufferOffset = skin.globSkinnedBoneBufferOffset;

    _pool->WriteToStaging(skeleton);
}