// graphics_backend2.hpp

#pragma once

#include <vector>
#include "backend.hpp"
#include "input_window_backend.hpp"
#include "graphics_device_type_enum.hpp"

namespace triton
{
    class cContext;

	class IGraphicsBackend2 : public iBackend
	{
        TRITON_CLASS_NAME(IGraphicsBackend2)

	public:
        explicit IGraphicsBackend2(cContext* context) : iBackend(context) {}
		~IGraphicsBackend2() override = default;

		// Initialization/Shutdown
		virtual void Initialize(
			SWindowBackend& window,
			types::boolean bEnableDebugging,
			const std::vector<const char*> extensions,
			EGraphicsDeviceType deviceType
		) = 0;

		virtual void Shutdown() = 0;
	};
}