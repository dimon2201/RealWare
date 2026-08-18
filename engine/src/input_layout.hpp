// input_layout.hpp

#pragma once

#include "object.hpp"
#include "handle.hpp"
#include "vertex_buffer_format.hpp"
#include "graphics_backend.hpp"

namespace triton
{
	class cContext;

	class XInputLayout : public iObject
	{
		TRITON_OBJECT(XInputLayout)

		CGPUInputLayout _gpuVertexArray = CGPUInputLayout(nullptr, 0, 0);

	public:
		explicit XInputLayout(
			cContext* context,
			const std::vector<CGPUBuffer>& buffersToBind,
			EVertexBufferFormat vertexFormat
		);
		~XInputLayout() override;

		struct THandle : public SHandle {};

		struct TGPULayout {};
	};
}