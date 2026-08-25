// shader_stage_enum.hpp

#pragma once

#include "types.hpp"

namespace triton
{
	enum class EShaderStage : types::dword
	{
		Undefined,
		Vertex,
		TessellationControl,
		TessellationEvaluate,
		Pixel,
		Compute
	};
}