// render_native_command_enum.hpp

#pragma once

#include "types.hpp"

namespace triton
{
	enum class ENativeRenderCommand : types::dword
	{
		Unknown,
		BeginRenderPass,
		BindPipeline,
		SetViewport,
		SetScissor,
		Draw,
		EndRenderPass,
		BindVertexBuffer,
		BindIndexBuffer
	};
}