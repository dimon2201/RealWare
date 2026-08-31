// gpu_buffer_types.hpp

#pragma once

namespace triton
{
	enum class EGPUBufferType
	{
		Unknown,
		Vertex,
		Index,
		Uniform,
		Storage,
		Staging
	};
}