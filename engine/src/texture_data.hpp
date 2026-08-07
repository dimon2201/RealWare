// texture_data.hpp

#pragma once

#include "math.hpp"
#include "handle.hpp"
#include "types.hpp"

namespace triton
{
	class cTexture;

	struct STextureData
	{
		struct THandle : public SHandle {};

		struct TGPULayout {};

		cTexture* gpuTexture = nullptr;
		cVector2 offsetNorm = cVector2(0.0f);
		cVector2 sizeNorm = cVector2(0.0f);
		cVector2 offsetPixel = cVector2(0.0f);
		cVector2 sizePixel = cVector2(0.0f);
		types::u32 zAtlasLayer = 0;
	};
}