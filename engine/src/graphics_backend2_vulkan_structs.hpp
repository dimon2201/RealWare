// graphics_backend2_vulkan_structs.hpp

#pragma once

#include <vector>
#include <vulkan/vulkan.h>
#include "math.hpp"
#include "graphics_device_type_enum.hpp"

namespace triton
{
	struct SImageWithView
	{
		SImageWithView(
			VkFormat format,
			VkImage image,
			VkImageView view
		) :
			format(format),
			image(image),
			view(view) {}

		VkFormat format = VK_FORMAT_UNDEFINED;
		VkImage image = VK_NULL_HANDLE;
		VkImageView view = VK_NULL_HANDLE;
	};
	
	struct SInstance
	{
		VkInstance instance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
	};

	struct SSurface
	{
		VkSurfaceKHR surface = VK_NULL_HANDLE;
	};

	struct SPhysicalDevice
	{
		SPhysicalDevice(
			EGraphicsDeviceType type,
			VkPhysicalDevice device,
			VkPhysicalDeviceProperties properties,
			VkPhysicalDeviceFeatures features,
			VkPhysicalDeviceFeatures2 features2,
			VkPhysicalDeviceVulkan13Features featuresVulkan13,
			const std::vector<VkQueueFamilyProperties>& queueFamilyProperties,
			VkSurfaceCapabilitiesKHR surfaceCapabilities
		) :
			type(type),
			device(device),
			properties(properties),
			features(features),
			features2(features2),
			featuresVulkan13(featuresVulkan13),
			queueFamilyProperties(queueFamilyProperties),
			surfaceCapabilities(surfaceCapabilities) {}

		EGraphicsDeviceType type;
		VkPhysicalDevice device = VK_NULL_HANDLE;
		VkPhysicalDeviceProperties properties;
		VkPhysicalDeviceFeatures features;
		VkPhysicalDeviceFeatures2 features2;
		VkPhysicalDeviceVulkan13Features featuresVulkan13;
		std::vector<VkQueueFamilyProperties> queueFamilyProperties;
		VkSurfaceCapabilitiesKHR surfaceCapabilities;
	};

	struct SQueue
	{
		SQueue(
			types::usize familyIndex,
			types::usize queueIndex,
			VkQueue queue
		) :
			familyIndex(familyIndex),
			queueIndex(queueIndex),
			queue(queue) {}

		types::usize familyIndex;
		types::usize queueIndex;
		VkQueue queue = VK_NULL_HANDLE;
		VkCommandPool commandPool = VK_NULL_HANDLE;
		VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
	};

	struct SLogicalDevice
	{
		VkDevice device = VK_NULL_HANDLE;
	};

	struct SSwapchain
	{
		cVector2 size = cVector2(0.0f);
		VkFormat format = VK_FORMAT_UNDEFINED;
		VkSwapchainKHR swapchain = VK_NULL_HANDLE;
		std::vector<SImageWithView> images;
	};

	struct SSemaphore
	{
		VkSemaphore semaphore = VK_NULL_HANDLE;
	};

	struct SFence
	{
		VkFence fence = VK_NULL_HANDLE;
	};
}