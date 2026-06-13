// instance_buffer.hpp

#pragma once

#include "graphics_resource_backend.hpp"
#include "types.hpp"

namespace triton
{
	class XInstanceBuffer final : public cBuffer
	{
		TRITON_OBJECT(XInstanceBuffer)

	public:
		explicit XInstanceBuffer(cContext* context, types::qword instance, eType type, types::usize byteSize, types::s32 slot)
			: cBuffer(context, instance, type, byteSize, slot) {}
		virtual ~XInstanceBuffer() override = default;
	};
}