// graphics_backend2_vulkan.hpp

#pragma once

#include <vector>
#include <vulkan/vulkan.h>
#include "object.hpp"
#include "graphics_backend2.hpp"
#include "graphics_backend2_vulkan_structs.hpp"

namespace triton
{
	class BGraphicsBackend2Vulkan : public IGraphicsBackend2
	{
		TRITON_CLASS_NAME(BGraphicsBackend2Vulkan)

		SInstance			_instance;
		SSurface			_surface;
		SPhysicalDevice		_physicalDevice = SPhysicalDevice(
			EGraphicsDeviceType::Unknown,
			VK_NULL_HANDLE,
			{},
			{},
			{},
			{},
			std::vector<VkQueueFamilyProperties>(),
			{}
		);

	public:
		explicit BGraphicsBackend2Vulkan(cContext* context) : IGraphicsBackend2(context) {}
		~BGraphicsBackend2Vulkan() override = default;

		// Initialization/Shutdown
		void Initialize(
			SWindowBackend& window,
			types::boolean bEnableDebugging,
			const std::vector<const char*> extensions,
			EGraphicsDeviceType deviceType
		) override final;

		void Shutdown() override final;
		
	private:
		void CreateInstance(types::boolean bEnableDebugging, const std::vector<const char*> extensions);
		void DestroyInstance();
		void CreateDebugMessenger();
		void DestroyDebugMessenger();
		void CreateSurface(SWindowBackend& window);
		void DestroySurface();
		void PickPhysicalDevice(EGraphicsDeviceType deviceType);
		void CheckPhysicalDeviceFeatures();
		void CheckPhysicalDeviceFeaturesVulkan13();
	};
}