// graphics_backend2_vulkan.hpp

#pragma once

#include <vector>
#include <vulkan/vulkan.h>
#include "object.hpp"
#include "graphics_backend2.hpp"
#include "graphics_backend2_vulkan_structs.hpp"
#include "texture_formats.hpp"

namespace triton
{
	class BGraphicsBackend2Vulkan : public IGraphicsBackend2
	{
		TRITON_CLASS_NAME(BGraphicsBackend2Vulkan)

		SInstance								_instance;
		SSurface								_surface;
		SPhysicalDevice							_physicalDevice = SPhysicalDevice(
			EGraphicsDeviceType::Unknown, VK_NULL_HANDLE, {}, {}, {}, {}, std::vector<VkQueueFamilyProperties>(), {}
		);
		SQueue									_graphicsQueue = SQueue(0, 0, {});
		SQueue									_transferQueue = SQueue(0, 0, {});
		SQueue									_computeQueue = SQueue(0, 0, {});
		SQueue									_presentQueue = SQueue(0, 0, {});
		SLogicalDevice							_logicalDevice;
		SSwapchain								_swapchain;
		std::vector<CGPURenderTargetResource>	_swapchainRenderTargets;
		CGPURenderPassResource					_swapchainRenderPass = CGPURenderPassResource(
			0, 0, 0
		);

	public:
		explicit BGraphicsBackend2Vulkan(cContext* context) : IGraphicsBackend2(context) {}
		~BGraphicsBackend2Vulkan() override = default;

		// Initialization/Shutdown
		void Initialize(
			SWindowBackend& window,
			types::boolean bEnableDebugging,
			const std::vector<const char*> extensions,
			EGraphicsDeviceType deviceType,
			const cVector2& swapchainSize
		) override final;

		void Shutdown() override final;

		CGPUTextureResource CreateTexture() override final;

		void DestroyTexture(CGPUTextureResource& renderTarget) override final;

		CGPURenderTargetResource CreateRenderTarget(
			const std::vector<CGPUTextureResource>& colorAttachments,
			const CGPUTextureResource& depthAttachment
		) override final;

		void DestroyRenderTarget(CGPURenderTargetResource& renderTarget) override final;

		CGPURenderPassResource CreateRenderPass(
			const CGPURenderTargetResource& renderTarget
		) override final;

		void DestroyRenderPass(CGPURenderPassResource& renderPass) override final;
		
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

		void FindQueueFamilies();

		void CreateLogicalDevice();

		void DestroyLogicalDevice();

		void CreateCommandPoolsAndCommandBuffers();

		void DestroyCommandPoolsAndCommandBuffers();

		void CreateSwapchain(const cVector2& size);

		void DestroySwapchain();

		VkSurfaceFormatKHR ChooseSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);

		VkPresentModeKHR ChooseSwapchainPresentMode(const std::vector<VkPresentModeKHR>& presentModes);

		VkExtent2D ChooseSwapchainExtent(
			VkSurfaceCapabilitiesKHR capabilities,
			const cVector2& size
		);

		void GetSwapchainImages();

		void CreateSwapchainRenderTarget();

		void DestroySwapchainRenderTarget();

		void CreateSwapchainRenderPass();

		void DestroySwapchainRenderPass();

		VkFormat TextureFormatToNative(ETextureFormat textureFormat);

		VkImageLayout AttachmentLayoutToNative(EGraphicsImageLayout attachmentLayout);
	};
}