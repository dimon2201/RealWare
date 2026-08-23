// graphics_backend2_vulkan.hpp

#pragma once

#include "graphics_backend2.hpp"

namespace triton
{
	class BGraphicsBackend2Vulkan : public IGraphicsBackend2
	{
	public:
		explicit BGraphicsBackend2Vulkan(cContext* context) : IGraphicsBackend2(context) {}
		~BGraphicsBackend2Vulkan() override = default;

		// Initialization
		virtual void Initialize(SWindowBackend& window) override final;
	};
}