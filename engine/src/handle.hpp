#pragma once

#include "types.hpp"

namespace triton
{
	class SHandle
	{
	protected:
		types::usize _index = 0;
		types::usize _generation = 0;
	};
}