// graphics_backend2.hpp

#pragma once

#include "backend.hpp"
#include "input_window_backend.hpp"

namespace triton
{
    class cContext;

	class IGraphicsBackend2 : public iBackend
	{
        TRITON_CLASS_NAME(IGraphicsBackend2)

	public:
        explicit IGraphicsBackend2(cContext* context) : iBackend(context) {}
		~IGraphicsBackend2() override = default;

		// Initialization
		virtual void Initialize(SWindowBackend& window) = 0;
	};
}