// render_resource_usage_enum.hpp

#pragma once

#include "types.hpp"

namespace triton
{
	enum class EResourceUsage : types::dword
	{
		Unknown,
		ColorAttachment,
		DepthAttachment,
		VertexShaderRead,
		PixelShaderRead,
		Present
	};
}