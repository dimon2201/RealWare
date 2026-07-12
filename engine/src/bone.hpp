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
		cMatrix4 offsetMatrix = cMatrix4(1.0f);
		cMatrix4 localMatrix = cMatrix4(1.0f);
		types::s32 parent = kNoParent;
		std::vector<types::usize> children = {};
	};
}