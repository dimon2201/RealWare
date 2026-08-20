// backend.hpp

#pragma once

namespace triton
{
	class cContext;

	class iBackend
	{
	protected:
		cContext* _context = nullptr;

	public:
		explicit iBackend(cContext* context) : _context(context) {}
		virtual ~iBackend() = default;
	};
}