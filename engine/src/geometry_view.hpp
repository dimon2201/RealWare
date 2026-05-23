#pragma once

#include "graphics_buffer_formats.hpp"
#include "types.hpp"

namespace triton
{
	class SGeometryView final
	{
	public:
		types::u32 _vertexCount = 0;
		types::u32 _indexCount = 0;
		types::u8* _vertexAddress = nullptr;
		types::u8* _indexAddress = nullptr;
		EGraphicsBufferFormat _format = EGraphicsBufferFormat::NONE;
	};
}