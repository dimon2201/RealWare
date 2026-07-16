#pragma once

#include "types.hpp"

namespace triton
{
	struct SSlot
	{
		types::usize _arrayIndex = 0;
		types::usize _generation = 0;
	};

	struct SHandle
	{
		static constexpr types::usize kInvalidValue = SIZE_MAX;

		types::usize _slotIndex = kInvalidValue;
		types::usize _indexInArray = kInvalidValue;
		types::usize _generation = kInvalidValue;
	};
}