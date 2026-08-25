// shader_stage_bit_enum.hpp

#pragma once

#include "types.hpp"

namespace triton
{
	enum class EShaderStageBit : types::dword
	{
		Undefined = 0,
		Vertex = 1,
		TessellationControl = 2,
		TessellationEvaluate = 4,
		Pixel = 8,
		Compute = 16
	};
}