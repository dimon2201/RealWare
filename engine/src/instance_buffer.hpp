// instance_buffer.hpp

#pragma once

#include "graphics_resource_backend.hpp"
#include "render_instance.hpp"
#include "types.hpp"

namespace triton
{
	class XDataBuffer;
	template <typename X, typename Y>
	class cHashTable;
	class cContext;

	class XInstanceBuffer final : public cBuffer
	{
		TRITON_OBJECT(XInstanceBuffer)

		XDataBuffer* _cpuBuffer = nullptr;
		cHashTable<std::string, types::usize>* _instances = nullptr;
		types::usize _firstDynamicInstanceBytePointer = 0;
		types::usize _lastDynamicInstanceBytePointer = 0;

	public:
		explicit XInstanceBuffer(cContext* context, types::qword instance, eType type, types::usize byteSize, types::s32 slot)
			: cBuffer(context, instance, type, byteSize, slot) {}
		virtual ~XInstanceBuffer() override = default;

		void Initialize();
		void Free();
		void Add(const std::string& tag, SRenderInstance::EUsage usage, const SRenderInstance& instance);
		void Remove(const std::string& tag);
		void Write(SRenderInstance::EUsage usage);
	};
}