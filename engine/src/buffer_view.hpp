// buffer_view.hpp

#pragma once

#include "types.hpp"

namespace triton
{
	template <typename TValue>
	class SBufferView
	{
	public:
		SBufferView(
			TValue* elements,
			types::usize byteSize
		) : elements(elements),
			byteSize(byteSize),
			elementCount(byteSize / sizeof(TValue)) {}

		TValue* elements = nullptr;
		types::usize byteSize = 0;
		types::usize elementCount = 0;
	};
}