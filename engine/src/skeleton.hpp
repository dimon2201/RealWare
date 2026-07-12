// skeleton.hpp

#pragma once

#include <vector>

namespace triton
{
	struct SBone;

	struct SSkeleton
	{
		std::vector<SBone> bones = {};
	};
}