// subsystem.hpp

#pragma once

#include "object.hpp"

namespace triton
{
	class cContext;

	class CSubsystem
	{
		TRITON_CLASS_NAME(CSubsystem)

	protected:
		cContext* _context = nullptr;

	public:
		explicit CSubsystem(cContext* context) : _context(context) {}
		virtual ~CSubsystem() = default;
	};
}