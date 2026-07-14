// graphics_buffer_formats.hpp

#pragma once

#include "types.hpp"

namespace triton
{
	enum class EGraphicsBufferFormat
	{
		NONE,
		POSITION_TEXCOORD_NORMAL_TANGENT_VEC3_VEC2_VEC3_VEC4,
		POS_TEX_NOR_TAN_MAT_BID_BWG_VEC3_VEC2_VEC3_VEC4_INT_UVEC4_VEC4
	};
}