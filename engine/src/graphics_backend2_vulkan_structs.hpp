// graphics_backend2_vulkan_structs.hpp

#pragma once

#include <vulkan/vulkan.h>
#include "graphics_device_type_enum.hpp"

namespace triton
{
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
		VkQueue queue;
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
		VkPhysicalDevice device;
		VkPhysicalDeviceProperties properties;
		VkPhysicalDeviceFeatures features;
		VkPhysicalDeviceFeatures2 features2;
		VkPhysicalDeviceVulkan13Features featuresVulkan13;
		std::vector<VkQueueFamilyProperties> queueFamilyProperties;
		VkSurfaceCapabilitiesKHR surfaceCapabilities;
	};

	struct SLogicalDevice
	{
		VkDevice device;
	};
}