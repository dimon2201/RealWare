// texture_data.hpp

#pragma once

#include "math.hpp"
#include "handle.hpp"
#include "gpu_texture.hpp"
#include "types.hpp"

namespace triton
{
	struct STextureData
	{
		struct THandle : public SHandle {};

		struct TGPULayout {};

		CGPUTexture gpuTexture;
		cVector2 offsetNorm = cVector2(0.0f);
		cVector2 sizeNorm = cVector2(0.0f);
		cVector2 offsetPixel = cVector2(0.0f);
		cVector2 sizePixel = cVector2(0.0f);
		types::u32 zAtlasLayer = 0;
	};
}