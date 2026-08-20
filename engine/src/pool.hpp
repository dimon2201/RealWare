// pool.hpp

#pragma once

#include "object.hpp"

namespace triton
{
	class cContext;

	class CPool
	{
		TRITON_CLASS_NAME(CPool)

	protected:
		cContext* _context = nullptr;

	public:
		explicit CPool(cContext* context) : _context(context) {}
		virtual ~CPool() = default;
	};
}