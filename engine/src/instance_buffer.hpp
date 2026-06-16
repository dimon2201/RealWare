// instance_buffer.hpp

#pragma once

#include "graphics_resource_backend.hpp"
#include "render_instance.hpp"
#include "tag.hpp"
#include "render_data.hpp"
#include "types.hpp"

namespace triton
{
	class XDataBuffer;
	template <typename X, typename Y>
	class cHashTable;
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

	class XInstanceBuffer final : public cBuffer
	{
		TRITON_OBJECT(XInstanceBuffer)

		XDataBuffer* _cpuBuffer = nullptr;
		cHashTable<cTag, SInstanceBufferOffset>* _instances = nullptr;
		types::usize _firstDynamicInstanceBytePointer = 0;
		types::usize _lastDynamicInstanceBytePointer = 0;

	public:
		explicit XInstanceBuffer(cContext* context, cBuffer* buffer)
			: cBuffer(context, buffer->GetInstance(), buffer->GetBufferType(), buffer->GetByteSize(), buffer->GetSlot()) {}
		virtual ~XInstanceBuffer() override = default;

		void Initialize();
		void Free();
		void Add(const std::string& tag, SRenderInstance::EUsage usage, const SRenderInstance& instance);
		void Remove(const std::string& tag);
		void UploadStatic(const SRenderData& data);
		void UploadDynamic(const SRenderData& data);
		void WriteToGPUStatic();
		void WriteToGPUDynamic();
	};
}