// graphics_backend2_vulkan.hpp

#pragma once

#include "graphics_backend2.hpp"
#include "graphics_backend2_vulkan_structs.hpp"

namespace triton
{
	class BGraphicsBackend2Vulkan : public IGraphicsBackend2
	{
		SInstance _instance;

	public:
		explicit BGraphicsBackend2Vulkan(cContext* context) : IGraphicsBackend2(context) {}
		~BGraphicsBackend2Vulkan() override = default;

		// Initialization/Shutdown
		void Initialize(
			types::boolean bEnableDebugging,
			void* specificData
		) override final;

		void Shutdown() override final;
		
	private:
		void CreateInstance(types::boolean bEnableDebugging);
		void DestroyInstance();
	};
}