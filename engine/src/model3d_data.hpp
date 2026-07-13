// model3d_data.hpp

#pragma once

#include <vector>
#include "vertex.hpp"
#include "handles.hpp"

namespace triton
{
	struct SModel3DData final
	{
        const SVertex* vertexData = nullptr;
        const types::u32* indexData = nullptr;
        types::usize vertexCount = 0;
        types::usize indexCount = 0;
        std::vector<HMaterial> materials = {};
        std::vector<HAnimation> animations = {};
	};
}