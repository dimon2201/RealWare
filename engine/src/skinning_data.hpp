// skinned_bone_data.hpp

#pragma once

#include "math.hpp"

namespace triton
{
	struct SGPUSkinningLayout
	{
		cMatrix4 modelMatrix = cMatrix4();
	};

	struct SSkinningData
	{
		cMatrix4 modelMatrix = cMatrix4();
	};
}