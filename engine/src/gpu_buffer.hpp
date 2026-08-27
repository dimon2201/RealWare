// gpu_buffer.hpp

#pragma once

#include "object.hpp"
#include "gpu_buffer_resource.hpp"
#include "handle.hpp"

namespace triton
{
	class cContext;

	class XGPUBuffer : public iObject
	{
		TRITON_OBJECT(XGPUBuffer)

		CGPUBufferResource _gpuBuffer = CGPUBufferResource::Invalid();

	public:
		explicit XGPUBuffer(
			cContext* context,
			types::s32 poolIndex,
			EGPUBufferType type,
			const types::u8* data,
			types::usize byteSize,
			types::s32 slot
		);

		~XGPUBuffer() override;

		void Write(types::usize offset, const types::u8* data, types::usize byteSize);

		inline const CGPUBufferResource& GetGPUResource() const { return _gpuBuffer; }

		struct THandle : public SHandle {};

		struct TGPULayout {};
	};
}