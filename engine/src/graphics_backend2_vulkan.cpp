// graphics_backend2_vulkan.cpp

#include <cstring>
#include <set>
#include "graphics_backend2_vulkan.hpp"
#include "input_backend.hpp"
#include "context.hpp"
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

boolean IsZeroTerminated(const char* str, usize maxLength)
{
	if (!str)
		return False;

	for (usize i = 0; i < maxLength; ++i)
		if (str[i] == '\0')
			return True;

	return False;
}

void triton::BGraphicsBackend2Vulkan::Initialize(
	SWindowBackend& window,
	boolean bEnableDebugging,
	const std::vector<const char*> extensions,
	EGraphicsDeviceType deviceType
)
{
	CreateInstance(bEnableDebugging, extensions);
	CreateSurface(window);
	PickPhysicalDevice(deviceType);
	CreateLogicalDevice();
}

void triton::BGraphicsBackend2Vulkan::Shutdown()
{
	DestroyLogicalDevice();
	DestroySurface();
	DestroyInstance();
}

void triton::BGraphicsBackend2Vulkan::CreateInstance(
	boolean bEnableDebugging,
	const std::vector<const char*> extensions
)
{
	if (bEnableDebugging == True &&
		!CheckValidationLayerSupport())
	{
		Print("Error: Vulkan validation layer is not available");
		return;
	}

	std::vector<const char*> validExtensions;
	for (usize i = 0; i < extensions.size(); i++)
		if (IsZeroTerminated(extensions[i], 1024))
			validExtensions.push_back(extensions[i]);

	validExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

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
	createInfo.enabledExtensionCount = (uint32_t)(validExtensions.size());
	createInfo.ppEnabledExtensionNames = validExtensions.data();
	createInfo.enabledLayerCount = 1;
	createInfo.ppEnabledLayerNames = &kValidationLayer;

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

	if (bEnableDebugging)
		CreateDebugMessenger();
}

void triton::BGraphicsBackend2Vulkan::DestroyInstance()
{
	DestroyDebugMessenger();
	
	if (_instance.instance != VK_NULL_HANDLE)
	{
		vkDestroyInstance(
			_instance.instance,
			nullptr
		);
	}
}

void triton::BGraphicsBackend2Vulkan::CreateDebugMessenger()
{
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
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

	PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessenger = VK_NULL_HANDLE;
	vkCreateDebugUtilsMessenger = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
		_instance.instance,
		"vkCreateDebugUtilsMessengerEXT"
	);

	VkResult result = vkCreateDebugUtilsMessenger(
		_instance.instance,
		&debugCreateInfo,
		nullptr,
		&_instance.debugMessenger
	);

	if (result != VK_SUCCESS)
		Print("Error: failed to create Vulkan debug messenger");
}

void triton::BGraphicsBackend2Vulkan::DestroyDebugMessenger()
{
	if (_instance.debugMessenger != VK_NULL_HANDLE)
	{
		PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessenger = VK_NULL_HANDLE;
		vkDestroyDebugUtilsMessenger = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
			_instance.instance,
			"vkDestroyDebugUtilsMessengerEXT"
		);

		vkDestroyDebugUtilsMessenger(
			_instance.instance,
			_instance.debugMessenger,
			nullptr
		);
	}
}

void triton::BGraphicsBackend2Vulkan::CreateSurface(SWindowBackend& window)
{
	void* surfaceRaw = _context->GetBackend<IInputBackend>()->CreateBackendWindowVulkanSurface(
		window,
		(void*)&_instance.instance
	);

	if (!surfaceRaw)
	{
		Print("Error: failed to create Vulkan surface");
		return;
	}

	_surface.surface = *((VkSurfaceKHR*)surfaceRaw);
}

void triton::BGraphicsBackend2Vulkan::DestroySurface()
{
	_context->GetBackend<IInputBackend>()->DestroyBackendWindowVulkanSurface(
		(void*)&_instance.instance,
		(void*)&_surface.surface
	);
}

void triton::BGraphicsBackend2Vulkan::PickPhysicalDevice(EGraphicsDeviceType deviceType)
{
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

	uint32_t deviceCount = 0;
	VkResult result = vkEnumeratePhysicalDevices(
		_instance.instance,
		&deviceCount,
		nullptr
	);

	if (result != VK_SUCCESS || deviceCount == 0)
	{
		Print("Error: no Vulkan physical devices found");
		return;
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	result = vkEnumeratePhysicalDevices(
		_instance.instance,
		&deviceCount,
		devices.data()
	);

	if (result != VK_SUCCESS)
	{
		Print("Error: failed to enumerate Vulkan physical devices");
		return;
	}

	for (VkPhysicalDevice device : devices)
	{
		VkPhysicalDeviceProperties properties = {};
		vkGetPhysicalDeviceProperties(
			device,
			&properties
		);

		if (deviceType == EGraphicsDeviceType::Discrete &&
			properties.deviceType ==
			VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			physicalDevice = device;
			break;
		}
		else if (deviceType == EGraphicsDeviceType::Integrated &&
			properties.deviceType ==
			VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
		{
			physicalDevice = device;
			break;
		}
	}

	if (physicalDevice == VK_NULL_HANDLE)
		physicalDevice = devices[0];

	VkPhysicalDeviceProperties properties = {};
	vkGetPhysicalDeviceProperties(
		physicalDevice,
		&properties
	);

	Print("[Vulkan] Picked device: " + std::string(properties.deviceName));

	uint32_t version = properties.apiVersion;
	uint32_t major = VK_VERSION_MAJOR(version);
	uint32_t minor = VK_VERSION_MINOR(version);
	uint32_t patch = VK_VERSION_PATCH(version);
	Print(
		"[Vulkan] API version: " +
		std::to_string(major) + "." +
		std::to_string(minor) + "." +
		std::to_string(patch)
	);

	EGraphicsDeviceType finalDeviceType = EGraphicsDeviceType::Unknown;

	if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		finalDeviceType = EGraphicsDeviceType::Discrete;
	else if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
		finalDeviceType = EGraphicsDeviceType::Integrated;
	
	VkPhysicalDeviceFeatures features = {};
	vkGetPhysicalDeviceFeatures(
		physicalDevice,
		&features
	);

	VkPhysicalDeviceFeatures2 features2 = {};
	features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	VkPhysicalDeviceVulkan13Features featuresVulkan13 = {};
	featuresVulkan13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
	features2.pNext = &featuresVulkan13;
	vkGetPhysicalDeviceFeatures2(
		physicalDevice,
		&features2
	);

	uint32_t queueCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(
		physicalDevice,
		&queueCount,
		nullptr
	);
	std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueCount);
	vkGetPhysicalDeviceQueueFamilyProperties(
		physicalDevice,
		&queueCount,
		queueFamilyProperties.data()
	);

	VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
		physicalDevice,
		_surface.surface,
		&surfaceCapabilities
	);

	_physicalDevice = SPhysicalDevice(
		finalDeviceType,
		physicalDevice,
		properties,
		features,
		features2,
		featuresVulkan13,
		queueFamilyProperties,
		surfaceCapabilities
	);

	CheckPhysicalDeviceFeatures();
	CheckPhysicalDeviceFeaturesVulkan13();
	FindQueueFamilies();
}

void triton::BGraphicsBackend2Vulkan::CheckPhysicalDeviceFeatures()
{
	if (_physicalDevice.features.multiDrawIndirect == VK_FALSE)
		Print("Error: Vulkan multi draw indirect is not supported");
}

void triton::BGraphicsBackend2Vulkan::CheckPhysicalDeviceFeaturesVulkan13()
{
	if (_physicalDevice.featuresVulkan13.synchronization2 == VK_FALSE)
		Print("Error: Vulkan synchronization2 is not supported");
	if (_physicalDevice.featuresVulkan13.dynamicRendering == VK_FALSE)
		Print("Error: Vulkan dynamicRendering is not supported");
}

void triton::BGraphicsBackend2Vulkan::FindQueueFamilies()
{
	for (usize i = 0; i < _physicalDevice.queueFamilyProperties.size(); i++)
	{
		VkQueueFamilyProperties queueFamilyProperties = _physicalDevice.queueFamilyProperties.at(i);
		if (queueFamilyProperties.queueCount > 0 &&
			queueFamilyProperties.queueFlags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT)
			_graphicsQueue = SQueue(i, 0, {});
		if (queueFamilyProperties.queueCount > 0 &&
			queueFamilyProperties.queueFlags & VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT)
			_transferQueue = SQueue(i, 0, {});
		if (queueFamilyProperties.queueCount > 0 &&
			queueFamilyProperties.queueFlags & VkQueueFlagBits::VK_QUEUE_COMPUTE_BIT)
			_computeQueue = SQueue(i, 0, {});

		VkBool32 presentSupported = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(
			_physicalDevice.device,
			(uint32_t)(i),
			_surface.surface,
			&presentSupported
		);
		if (presentSupported == VK_TRUE)
			_presentQueue = SQueue(i, 0, {});
	}
}

void triton::BGraphicsBackend2Vulkan::CreateLogicalDevice()
{
	std::set<usize> uniqueQueueFamilyIndices = {
		_graphicsQueue.familyIndex,
		_transferQueue.familyIndex,
		_computeQueue.familyIndex,
		_presentQueue.familyIndex
	};

	const float priority = 1.0f;

	usize counter = 0;
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos(uniqueQueueFamilyIndices.size());
	for (auto queueFamilyIndex : uniqueQueueFamilyIndices)
	{
		queueCreateInfos[counter].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfos[counter].queueCount = 1;
		queueCreateInfos[counter].queueFamilyIndex = (uint32_t)queueFamilyIndex;
		queueCreateInfos[counter].pQueuePriorities = &priority;
		++counter;
	}

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = (uint32_t)queueCreateInfos.size();
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	VkResult result = vkCreateDevice(
		_physicalDevice.device,
		&createInfo,
		nullptr,
		&_logicalDevice.device
	);

	if (result != VK_SUCCESS)
		Print("Error: failed to create Vulkan logical device");
}

void triton::BGraphicsBackend2Vulkan::DestroyLogicalDevice()
{
	if (_logicalDevice.device != VK_NULL_HANDLE)
		vkDestroyDevice(_logicalDevice.device, nullptr);
}