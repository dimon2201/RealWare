// skeleton.hpp

#pragma once

#include <vector>
#include "bone.hpp"
#include "math.hpp"

namespace triton
{
	struct SGPUSkeletonLayout
	{
		types::u32 globSkinnedBoneBufferOffset;
	};

	struct SSkeletonData
	{
		types::u32 globSkinnedBoneBufferOffset = 0;
		cMatrix4 accumulatedRootTransform = cMatrix4();
		std::vector<SBone> bones = {};
	};
}