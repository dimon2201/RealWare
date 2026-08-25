// graphics_image_layout.hpp

#pragma once

#include "types.hpp"

namespace triton
{
	enum class EGraphicsImageLayout : types::u32
	{
		Undefined,
		ColorAttachment,
		DepthStencilAttachment,
		ShaderRead,
		Present
	};
}