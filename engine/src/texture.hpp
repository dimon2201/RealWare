// texture.hpp

#pragma once

#include "math.hpp"

namespace triton
{
	class cTexture;

	struct STexture
	{
		cTexture* gpuInstance = nullptr;
		cVector2 size = cVector2(0.0f);
	};
}