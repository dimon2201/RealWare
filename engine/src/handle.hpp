#pragma once

#include "types.hpp"

namespace triton
{
	class SSlot
	{
	protected:
		void* _object = nullptr;
		types::usize _generation = 0;
	};

	class SHandle
	{
	protected:
		types::usize _index = 0;
		types::usize _generation = 0;
	};
}