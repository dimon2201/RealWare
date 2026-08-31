// render_native_command_struct.hpp

#pragma once

#include "render_native_command_enum.hpp"
#include "types.hpp"

namespace triton
{
	struct SNativeRenderCommand final
	{
		ENativeRenderCommand cmd = ENativeRenderCommand::Unknown;
		types::qword args[8] = {};
	};
}