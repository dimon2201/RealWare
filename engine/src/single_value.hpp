#pragma once

#include "stack_value.hpp"
#include "types.hpp"

namespace triton
{
	class cSingleValue : public cStackValue
	{
		types::qword _value = 0;

	public:
		cSingleValue(types::qword value) : _value(value) {}

		inline types::qword Value() const { return _value; }
	};
}