// graphics_backend2_vulkan.hpp

#pragma once

#include "object.hpp"
#include "graphics_backend2.hpp"
#include "graphics_backend2_vulkan_structs.hpp"

namespace triton
{
	class BGraphicsBackend2Vulkan : public IGraphicsBackend2
	{
		TRITON_CLASS_NAME(BGraphicsBackend2Vulkan)

		SInstance _instance;

	public:
		explicit BGraphicsBackend2Vulkan(cContext* context) : IGraphicsBackend2(context) {}
		~BGraphicsBackend2Vulkan() override = default;

		// Initialization/Shutdown
		void Initialize(
			types::boolean bEnableDebugging,
			const std::vector<const char*> extensions
		) override final;

		void Shutdown() override final;
		
	private:
		void CreateInstance(types::boolean bEnableDebugging, const std::vector<const char*> extensions);
		void DestroyInstance();
		void CreateDebugMessenger();
		void DestroyDebugMessenger();
	};
}