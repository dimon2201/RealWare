// graphics_texture_usage_enum.hpp

#pragma once

#include "types.hpp"

namespace triton
{
	enum class ETextureUsageBit : types::dword
	{
		Undefined,
		Sampled = 1,
		ColorAttachment = 2,
		DepthStencilAttachment = 4
	};
}