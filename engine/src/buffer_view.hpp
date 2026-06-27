// buffer_view.hpp

#pragma once

#include "types.hpp"

namespace triton
{
	template <typename TValue>
	class SBufferView
	{
	public:
		SBufferView(TValue* elements, types::usize byteSize) : _elements(elements), _byteSize(byteSize), _elementCount(byteSize / sizeof(TValue)) {}

		TValue* _elements = nullptr;
		types::usize _byteSize = 0;
		types::usize _elementCount = 0;
	};
}