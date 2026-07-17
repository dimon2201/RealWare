// TODO: remove this as soon as possible
// ||||||||||||||||||||||||||||||||||||||
// VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV

#pragma once

#include "object.hpp"

namespace triton
{
	class cContext;
	class ISubsys : public iObject
	{
	public:
		explicit ISubsys(cContext* context) : iObject(context) {}
		~ISubsys() override = default;

		virtual void Init() = 0;
		virtual void Free() = 0;
		virtual void Update() = 0;
	};
}

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// ||||||||||||||||||||||||||||||||||||||