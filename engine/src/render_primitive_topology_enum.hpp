// render_primitive_topology_enum.hpp

#pragma once

#include "types.hpp"

namespace triton
{
	enum class EPrimitiveTopology : types::dword
	{
		Unknown,
		TriangleList,
		TriangleStrip
	};
}