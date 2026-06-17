// instance_buffer.hpp

#pragma once

#include "graphics_resource_backend.hpp"
#include "render_instance.hpp"
#include "tag.hpp"
#include "render_data.hpp"
#include "handle.hpp"
#include "types.hpp"

namespace triton
{
	class XDataBuffer;
	template <typename X, typename Y, typename Z>
	class XHandleAllocator;
	class cContext;

	class SInstanceBufferOffset final : public cStackValue
	{
		types::usize _offset = 0;

	public:
		SInstanceBufferOffset() = default;
		SInstanceBufferOffset(types::usize offset) : _offset(offset) {}

		inline types::usize GetOffset() const
		{
			return _offset;
		}
	};

	class SInstanceBufferSlot final : public SSlot {};

	class SInstanceBufferHandle final : public SHandle {};

	class XInstanceBuffer final : public cBuffer
	{
		TRITON_OBJECT(XInstanceBuffer)

		XDataBuffer* _cpuBuffer = nullptr;
		types::usize _firstDynamicInstanceBytePointer = 0;
		types::usize _lastDynamicInstanceBytePointer = 0;
		XHandleAllocator<SInstanceBufferSlot, SInstanceBufferHandle, SInstanceBufferOffset>* _handles = nullptr;

	public:
		explicit XInstanceBuffer(cContext* context, cBuffer* buffer)
			: cBuffer(context, buffer->GetInstance(), buffer->GetBufferType(), buffer->GetByteSize(), buffer->GetSlot()) {}
		virtual ~XInstanceBuffer() override = default;

		void Initialize();
		void Free();
		SInstanceBufferHandle Add(const SRenderInstance& instance);
		SRenderInstance* Get(const SInstanceBufferHandle& handle);
		void Remove(const SInstanceBufferHandle& handle);
		void UploadStatic(const SRenderData& data);
		void UploadDynamic(const SRenderData& data);
		void WriteToGPUStatic();
		void WriteToGPUDynamic();
	};
}