// graphics_backend2_vulkan_structs.hpp

#pragma once

#include <vulkan/vulkan.h>

namespace triton
{
	struct SInstance
	{
		VkInstance instance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
	};

	struct SSurface
	{
		VkSurfaceKHR surface = VK_NULL_HANDLE;
	};
}