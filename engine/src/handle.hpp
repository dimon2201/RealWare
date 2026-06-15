#pragma once

#include "types.hpp"

namespace triton
{
	class SSlot
	{
	public:
		types::usize _arrayIndex = 0;
		types::usize _generation = 0;
	};

	class SHandle
	{
	public:
		types::usize _slotIndex = 0;
		types::usize _generation = 0;
	};
}