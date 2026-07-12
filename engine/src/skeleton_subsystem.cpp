// skeleton_subsystem.cpp

#include "skeleton_subsystem.hpp"
#include "skeleton.hpp"

using namespace types;

triton::HSkeleton triton::XSkeletonSubsystem::CreateSkeleton(const std::vector<SBone>& bones)
{
    HSkeleton skeleton = Create();
    SSkeleton& s = Get(skeleton);
    s.bones = bones;

    return skeleton;
}

void triton::XSkeletonSubsystem::DestroySkeleton(const HSkeleton& skeleton)
{
    Destroy(skeleton);
}