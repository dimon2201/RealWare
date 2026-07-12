// skeleton.hpp

#pragma once

#include <vector>

namespace triton
{
	struct SBone;

	struct SSkeleton
	{
		types::usize globBoneOffset = 0;
		std::vector<SBone> bones = {};
	};
}