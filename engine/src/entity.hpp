#pragma once

#include "types.hpp"

namespace triton::ecs
{
	using entity = types::u64;
	static constexpr entity kInvalidEntity = 0;
	static types::u64 sLastEntity = kInvalidEntity;
}