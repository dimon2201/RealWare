// image_format_enum.hpp

#pragma once

#include "types.hpp"

namespace triton
{
	enum class EImageFormat : types::u32
	{
		Unknown,
		R8,
		RGBA8,
		RGBA8_SRGB
	};
}