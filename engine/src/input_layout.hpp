// input_layout.hpp

#pragma once

#include "object.hpp"
#include "handle.hpp"
#include "vertex_buffer_format.hpp"
#include "graphics_pipeline_backend.hpp"

namespace triton
{
	class cContext;
	class cBuffer;

	class XInputLayout : public iObject
	{
		TRITON_OBJECT(XInputLayout)

		CGPUVertexArray _gpuVertexArray = CGPUVertexArray(nullptr, 0);

	public:
		explicit XInputLayout(
			cContext* context,
			const std::vector<cBuffer*>& buffersToBind,
			EVertexBufferFormat vertexFormat
		);
		~XInputLayout() override;

		struct THandle : public SHandle {};

		struct TGPULayout {};
	};
}