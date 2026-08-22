// input_key_code_enum.hpp

#pragma once

#include "types.hpp"

namespace triton
{
	enum class EKeyCode : types::u8
	{
		Unknown,
		MouseLeft,
		MouseRight,
		MouseMiddle,
		W,
		A,
		S,
		D
	};
}