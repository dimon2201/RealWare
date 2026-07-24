#pragma once

#include "types.hpp"

namespace triton
{
	struct SSlot
	{
		types::usize _arrayIndex = 0;
		types::usize _generation = 0;
		types::boolean _alive = types::K_FALSE;
	};

	struct SHandle
	{
		static constexpr types::usize kInvalidValue = SIZE_MAX;

		SHandle()
		{
			Invalidate();
		}

		void Invalidate()
		{
			_slotIndex = kInvalidValue;
			_indexInArray = kInvalidValue;
			_generation = kInvalidValue;
		}

		types::boolean IsInvalid() const
		{
			return (_slotIndex == kInvalidValue &&
				   _indexInArray == kInvalidValue &&
				   _generation == kInvalidValue)
					== true ? types::K_TRUE : types::K_FALSE;
		}

		types::usize _slotIndex;
		types::usize _indexInArray;
		types::usize _generation;
	};

	struct SInvalidHandle : SHandle {};
}