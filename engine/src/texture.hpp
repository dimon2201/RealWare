// texture.hpp

#pragma once

#include "math.hpp"
#include "types.hpp"

namespace triton
{
	class cTexture;

	struct STexture
	{
		cTexture* gpuInstance = nullptr;
		types::u32 layer = 0;
		cVector2 normOffset = cVector2(0.0f);
		cVector2 normSize = cVector2(0.0f);
		cVector2 pixelOffset = cVector2(0.0f);
		cVector2 pixelSize = cVector2(0.0f);
	};
}