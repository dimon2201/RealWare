// skeleton.cpp

#include "skeleton.hpp"

triton::XSkeleton::XSkeleton(
	cContext* context,
	types::s32 poolIndex,
	const std::vector<SSkeletonBone>& bones,
	const cMatrix4& accumulatedRootTransform
) : iObject(context, poolIndex), _bones(bones), _accumulatedRootTransform(accumulatedRootTransform) {}