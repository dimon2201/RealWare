// graphics_backend2_vulkan.cpp

#include <cstring>
#include "graphics_backend2_vulkan.hpp"
#include "log.hpp"

using namespace types;

constexpr const char* kValidationLayer = "VK_LAYER_KHRONOS_validation";

types::boolean CheckValidationLayerSupport()
{
	uint32_t layerCount = 0;

	VkResult result = vkEnumerateInstanceLayerProperties(
		&layerCount,
		nullptr
	);

	if (result != VK_SUCCESS)
		return False;

	std::vector<VkLayerProperties> availableLayers(layerCount);

	result = vkEnumerateInstanceLayerProperties(
		&layerCount,
		availableLayers.data()
	);

	if (result != VK_SUCCESS)
		return False;

	for (const VkLayerProperties& layer : availableLayers)
	{
		if (std::strcmp(layer.layerName, kValidationLayer) == 0)
			return True;
	}

	return False;
}

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
	void* userData
)
{
	triton::Print(
		"[Vulkan] " +
		std::string(callbackData->pMessage)
	);

	return VK_FALSE;
}

VkResult CreateDebugMessenger(
	VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* createInfo,
	const VkAllocationCallbacks* allocator,
	VkDebugUtilsMessengerEXT* debugMessenger
)
{
	auto function =
		(PFN_vkCreateDebugUtilsMessengerEXT)(
			vkGetInstanceProcAddr(
				instance,
				"vkCreateDebugUtilsMessengerEXT"
			)
		);

	if (function != nullptr)
	{
		return function(
			instance,
			createInfo,
			allocator,
			debugMessenger
		);
	}

	return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void DestroyDebugMessenger(
	VkInstance instance,
	VkDebugUtilsMessengerEXT debugMessenger,
	const VkAllocationCallbacks* allocator
)
{
	auto function =
		(PFN_vkDestroyDebugUtilsMessengerEXT)(
			vkGetInstanceProcAddr(
				instance,
				"vkDestroyDebugUtilsMessengerEXT"
			)
		);

	if (function != nullptr)
	{
		function(
			instance,
			debugMessenger,
			allocator
		);
	}
}

void triton::BGraphicsBackend2Vulkan::Initialize(
	boolean bEnableDebugging,
	void* data
)
{
	CreateInstance(bEnableDebugging);
}

void triton::BGraphicsBackend2Vulkan::CreateInstance(boolean bEnableDebugging)
{
	if (bEnableDebugging == True &&
		!CheckValidationLayerSupport())
	{
		Print("Error: Vulkan validation layer is not available");
		return;
	}

	std::vector<const char*> extensions;
	extensions.push_back(
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME
	);

	VkApplicationInfo applicationInfo = {};
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.pApplicationName = "TritonEngineApplication";
	applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	applicationInfo.pEngineName = "TritonEngine";
	applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	applicationInfo.apiVersion = VK_API_VERSION_1_3;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &applicationInfo;
	createInfo.enabledExtensionCount = (uint32_t)(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
	if (bEnableDebugging == True)
	{
		debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugCreateInfo.messageSeverity =
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugCreateInfo.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debugCreateInfo.pfnUserCallback =
			DebugCallback;

		createInfo.enabledLayerCount = 1;
		createInfo.ppEnabledLayerNames =
			&kValidationLayer;
		createInfo.pNext =
			&debugCreateInfo;
	}

	VkResult result = vkCreateInstance(
		&createInfo,
		nullptr,
		&_instance.instance
	);

	if (result != VK_SUCCESS)
	{
		Print("Error: failed to create Vulkan instance");
		return;
	}

	if (bEnableDebugging == True)
	{
		result = CreateDebugMessenger(
			_instance.instance,
			&debugCreateInfo,
			nullptr,
			&_instance.debugMessenger
		);

		if (result != VK_SUCCESS)
		{
			Print("Error: failed to create Vulkan debug messenger");
			return;
		}
	}
}