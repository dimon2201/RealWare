// memory_units.hpp

#pragma once

#include "types.hpp"

namespace triton
{
	constexpr types::usize KiB(types::usize count)
	{
		return count * 1024;
	}

	constexpr types::usize MiB(types::usize count)
	{
		return KiB(count) * 1024;
	}

	constexpr types::usize GiB(types::usize count)
	{
		return MiB(count) * 1024;
	}

	constexpr types::usize TiB(types::usize count)
	{
		return GiB(count) * 1024;
	}
}