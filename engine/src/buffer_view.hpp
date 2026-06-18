// buffer_view.hpp

#pragma once

#include "types.hpp"

namespace triton
{
	class SBufferView
	{
	public:
		SBufferView(void* data, types::usize byteSize) : _data(data), _byteSize(byteSize) {}

		void* _data = nullptr;
		types::usize _byteSize = 0;
	};
}