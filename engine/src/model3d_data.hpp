// model3d_data.hpp

#pragma once

#include <vector>
#include "vertex.hpp"
#include "animation.hpp"
#include "material_data.hpp"
#include "skeleton_data.hpp"

namespace triton
{
	struct SModel3DData final
	{
        struct THandle : public SHandle {};

        struct TGPULayout {};

        const SSkinnedVertexGPULayout* vertexData = nullptr;
        const types::u32* indexData = nullptr;
        types::usize vertexCount = 0;
        types::usize indexCount = 0;
        std::vector<SMaterialData::THandle> materials = {};
        SSkeletonData::THandle skeleton = {};
        std::vector<SAnimationData::THandle> animations = {};
	};
}