// input_layout.hpp

#pragma once

#include "object.hpp"
#include "handle.hpp"
#include "vertex_buffer_format.hpp"
#include "gpu_input_layout.hpp"
#include "gpu_buffer.hpp"

namespace triton
{
	class cContext;

	class XInputLayout : public iObject
	{
		TRITON_OBJECT(XInputLayout)

		CGPUInputLayoutResource _gpuInputLayout;

	public:
		explicit XInputLayout(
			cContext* context,
			const std::vector<XGPUBuffer::THandle>& buffersToBind,
			EVertexBufferFormat vertexFormat
		);
		~XInputLayout() override;

		inline const CGPUInputLayoutResource& GetGPUResource() const { return _gpuInputLayout; }

		struct THandle : public SHandle {};

		struct TGPULayout {};
	};
}