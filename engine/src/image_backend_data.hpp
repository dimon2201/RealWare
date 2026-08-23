// image_backend_data.hpp

#pragma once

#include "math.hpp"
#include "image_format_enum.hpp"
#include "types.hpp"

namespace triton
{
	struct SImageBackend
	{
		types::u8* data = nullptr;
		types::usize dataByteSize = 0;
		EImageFormat dataFormat = EImageFormat::Unknown;
		cVector3 size = cVector3(0.0f);
	};
}