// skeleton.hpp

#pragma once

#include <vector>
#include "bone.hpp"
#include "math.hpp"

namespace triton
{
	struct SSkeleton
	{
		types::u32 globSkinnedBoneOffset = 0;
		cMatrix4 accumulatedRootTransform = cMatrix4();
		std::vector<SBone> bones = {};
	};
}