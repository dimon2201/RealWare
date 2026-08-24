// texture_formats.hpp

#pragma once

namespace triton
{
	enum class ETextureFormat
	{
		Unknown,
		R8,
		R8F,
		RGBA8,
		RGBA8_SRGB,
		BGRA8_SRGB,
		RGB16F,
		RGBA16F,
		RGBA8_SRGB_Mips,
		DepthStencil
	};
}