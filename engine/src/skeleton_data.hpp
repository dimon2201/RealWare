// skeleton_data.hpp

#pragma once

#include <vector>
#include "bone.hpp"
#include "math.hpp"
#include "handle.hpp"

namespace triton
{
	struct SSkeletonData
	{
		struct THandle : public SHandle {};

		struct TGPULayout {};

		cMatrix4 accumulatedRootTransform = cMatrix4();
		std::vector<SBone> bones = {};
	};
}