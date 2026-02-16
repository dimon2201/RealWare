#pragma once

#include "types.hpp"

namespace triton
{
	class cHandle
	{
	protected:
		index idx = 0;
		types::usize generation = 0;

	public:
		using index = types::u32;
	};
}