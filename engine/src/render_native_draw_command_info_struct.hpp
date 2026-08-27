// render_native_draw_command_info_struct.hpp

#pragma once

#include "types.hpp"

namespace triton
{
	struct SNativeCommandDrawInfo final
	{
		types::usize vertexCount = 0;
		types::usize indexCount = 0;
		types::usize instanceCount = 0;
		types::usize firstIndex = 0;
		types::usize baseVertex = 0;
		types::usize firstInstance = 0;
	};
}