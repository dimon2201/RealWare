// bone.hpp

#pragma once

#include <vector>
#include <string>
#include "math.hpp"

namespace triton
{
	struct SBone
	{
		static constexpr types::s32 kNoParent = -1;
		static constexpr types::usize kMaxChildCount = 16;

		std::string name = {};
		cMatrix4 localMatrix = cMatrix4();
		cMatrix4 modelMatrix = cMatrix4();
		types::s32 localParentBoneIndex = kNoParent;
		std::vector<types::usize> localChildBoneIndices = {};
	};
}