// backend.hpp

#pragma once

#include "object.hpp"

namespace triton
{
	class cContext;

	class iBackend : public iObject
	{
		TRITON_OBJECT(iBackend)

	public:
		explicit iBackend(cContext* context);
		virtual ~iBackend() override = default;
	};
}