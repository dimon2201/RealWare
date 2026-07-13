// skeleton.hpp

#pragma once

#include <vector>
#include "bone.hpp"

namespace triton
{
	struct SSkeleton
	{
		types::u32 globSkinnedBoneOffset = 0;
		std::vector<SBone> bones = {};
	};
}