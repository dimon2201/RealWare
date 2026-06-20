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
	template <typename TSlot, typename THandle, typename TDataStructure, typename TObject>
	class XHandleAllocator;
	class cContext;
	template <typename TValue>
	class XLinearArray;
	class SRenderInstance;

	class SInstanceBufferOffset final
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

		XHandleAllocator<SInstanceBufferSlot, SInstanceBufferHandle, XLinearArray<SRenderInstance>, SRenderInstance>* _instances = nullptr;

	public:
		explicit XInstanceBuffer(cContext* context, cBuffer* buffer)
			: cBuffer(context, buffer->GetInstance(), buffer->GetBufferType(), buffer->GetByteSize(), buffer->GetSlot()) {}
		virtual ~XInstanceBuffer() override = default;

		void Initialize();
		void Free();
		SInstanceBufferHandle Add(SRenderInstance& instance);
		SRenderInstance* Get(SInstanceBufferHandle& handle);
		void Remove(SInstanceBufferHandle& handle);
		void Write(const SInstanceBufferHandle& handle);
		void WriteAll();
	};
}