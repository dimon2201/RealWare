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

	class SInstanceBufferHandle final : public SHandle
	{
	public:
		SRenderInstance::EUsage _usage = SRenderInstance::EUsage::NONE;

		SInstanceBufferHandle(SRenderInstance::EUsage usage) : SHandle(), _usage(usage) {}
	};

	class XInstanceBuffer final : public cBuffer
	{
		TRITON_OBJECT(XInstanceBuffer)

		SRenderInstance::EUsage _usage = SRenderInstance::EUsage::NONE;

	public:
		explicit XInstanceBuffer(cContext* context, SRenderInstance::EUsage usage);
		explicit XInstanceBuffer(cContext* context, SRenderInstance::EUsage usage, cBuffer* buffer);
		~XInstanceBuffer() override;
	};
}