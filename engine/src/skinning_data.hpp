// skinning_data.hpp

#pragma once

#include <vector>
#include "handle.hpp"
#include "math.hpp"

namespace triton
{
	struct SSkinnedBoneData
	{
		struct THandle : public SHandle {};

		struct TGPULayout
		{
			cMatrix4 modelMatrix = cMatrix4();
		};

		cMatrix4 modelMatrix = cMatrix4();
	};

	struct SSkinData
	{
		struct THandle : public SHandle {};

		struct TGPULayout {};

		std::vector<SSkinnedBoneData::THandle> skinnedBones;
	};
}