// skeleton_subsystem.cpp

#include "skeleton_subsystem.hpp"

using namespace types;

triton::HSkeleton triton::XSkeletonSubsystem::CreateAnimation()
{
    HSkeleton skeleton = Create();
    SSkeleton& s = Get(skeleton);

    return skeleton;
}

void triton::XSkeletonSubsystem::DestroySkeleton(const HSkeleton& skeleton)
{
    Destroy(skeleton);
}