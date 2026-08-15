#pragma once

#include "types.hpp"

namespace triton
{
	struct SSlot
	{
		types::boolean alive = types::K_FALSE;
		types::u32 generation = 0;
	};

	struct SHandle
	{
		types::usize index = 0;
		types::u32 generation = 0;
	};

	struct SInvalidHandle : SHandle {};
}