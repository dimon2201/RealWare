// graphics_backend2_vulkan.cpp

#include <cstring>
#include <set>
#include <algorithm>
#include <cstdint>
#include "graphics_backend2_vulkan.hpp"
#include "input_backend.hpp"
#include "context.hpp"
#include "object_allocator.hpp"
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
	EGraphicsDeviceType deviceType,
	const cVector2& swapchainSize
)
{
	CreateInstance(bEnableDebugging, extensions);
	CreateSurface(window);
	PickPhysicalDevice(deviceType);
	CreateLogicalDevice();
	CreateCommandPoolsAndCommandBuffers();
	CreateSwapchain(swapchainSize);
}

void triton::BGraphicsBackend2Vulkan::Shutdown()
{
	DestroySwapchain();
	DestroyCommandPoolsAndCommandBuffers();
	DestroyLogicalDevice();
	DestroySurface();
	DestroyInstance();
}

triton::CGPUTextureResource triton::BGraphicsBackend2Vulkan::CreateTexture()
{
	return CGPUTextureResource(0, 0, cVector3(0.0f), ETextureDimension::Unknown, ETextureFormat::Unknown, 0);
}

void triton::BGraphicsBackend2Vulkan::DestroyTexture(CGPUTextureResource& renderTarget)
{
}

triton::CGPURenderTargetResource triton::BGraphicsBackend2Vulkan::CreateRenderTarget(
	const std::vector<CGPUTextureResource>& colorAttachments,
	const CGPUTextureResource& depthAttachment
)
{
	void* instance = CObjectAllocator::Allocate(sizeof(cpuword), 64);

	return CGPURenderTargetResource(
		(cpuword)instance,
		0,
		colorAttachments,
		depthAttachment
	);
}

void triton::BGraphicsBackend2Vulkan::DestroyRenderTarget(CGPURenderTargetResource& renderTarget)
{
	if (renderTarget.IsValid())
	{
		CObjectAllocator::Deallocate((void*)renderTarget.GetInstance());

		renderTarget.Invalidate();
	}
}

triton::CGPURenderPassResource triton::BGraphicsBackend2Vulkan::CreateRenderPass(
	const CGPURenderTargetResource& renderTarget
)
{
	const usize colorAttachmentCount = renderTarget.GetColorAttachmentCount();
	const boolean hasDepth = renderTarget.GetDepthAttachment().IsValid();
	const usize depthAttachmentCount = hasDepth == True ? 1 : 0;

	std::vector<VkAttachmentDescription> attachments(colorAttachmentCount + depthAttachmentCount);
	std::vector<VkAttachmentReference> colorReferences(colorAttachmentCount);
	VkAttachmentReference depthReference = {};

	for (usize i = 0; i < colorAttachmentCount; ++i)
	{
		attachments[i].format = TextureFormatToNative(
			renderTarget.GetColorAttachments()[i].GetFormat()
		);
		attachments[i].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[i].initialLayout = AttachmentLayoutToNative(
			renderTarget.GetColorAttachmentSrcLayouts()[i]
		);
		attachments[i].finalLayout = AttachmentLayoutToNative(
			renderTarget.GetColorAttachmentDstLayouts()[i]
		);

		colorReferences[i].attachment = i;
		colorReferences[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}

	if (hasDepth)
	{
		attachments[colorAttachmentCount].format = TextureFormatToNative(
			renderTarget.GetDepthAttachment().GetFormat()
		);
		attachments[colorAttachmentCount].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[colorAttachmentCount].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[colorAttachmentCount].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[colorAttachmentCount].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[colorAttachmentCount].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[colorAttachmentCount].initialLayout = AttachmentLayoutToNative(
			renderTarget.GetDepthAttachmentSrcLayout()
		);
		attachments[colorAttachmentCount].finalLayout = AttachmentLayoutToNative(
			renderTarget.GetDepthAttachmentDstLayout()
		);

		depthReference.attachment = colorAttachmentCount;
		depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	}

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = (uint32_t)colorReferences.size();
	subpass.pColorAttachments = colorReferences.data();
	if (hasDepth)
		subpass.pDepthStencilAttachment = &depthReference;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.srcStageMask =
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstStageMask =
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask =
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	if (hasDepth)
	{
		dependency.dstStageMask |=
			VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

		dependency.dstAccessMask |=
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	}

	VkRenderPassCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	createInfo.attachmentCount = attachments.size();
	createInfo.pAttachments = attachments.data();
	createInfo.subpassCount = 1;
	createInfo.pSubpasses = &subpass;
	createInfo.dependencyCount = 1;
	createInfo.pDependencies = &dependency;

	VkRenderPass renderPass = VK_NULL_HANDLE;

	VkResult result = vkCreateRenderPass(
		_logicalDevice.device,
		&createInfo,
		nullptr,
		&renderPass
	);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to create render pass");

	std::vector<VkImageView> framebufferAttachments(colorAttachmentCount + depthAttachmentCount);
	for (usize i = 0; i < colorAttachmentCount; i++)
		framebufferAttachments[i] = (VkImageView)renderTarget.GetColorAttachments()[i].GetView();
	
	if (hasDepth == True)
		framebufferAttachments[colorAttachmentCount] = (VkImageView)renderTarget.GetDepthAttachment().GetView();

	VkFramebufferCreateInfo framebufferCreateInfo = {};
	framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferCreateInfo.renderPass = renderPass;
	framebufferCreateInfo.attachmentCount = (uint32_t)attachments.size();
	framebufferCreateInfo.pAttachments = framebufferAttachments.data();
	framebufferCreateInfo.width = renderTarget.GetSize().GetX();
	framebufferCreateInfo.height = renderTarget.GetSize().GetY();
	framebufferCreateInfo.layers = 1;

	VkFramebuffer framebuffer = VK_NULL_HANDLE;

	result = vkCreateFramebuffer(
		_logicalDevice.device,
		&framebufferCreateInfo,
		nullptr,
		&framebuffer
	);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to create render pass framebuffer");

	return CGPURenderPassResource(
		(types::qword)renderPass,
		0,
		(types::qword)framebuffer
	);
}

void triton::BGraphicsBackend2Vulkan::DestroyRenderPass(CGPURenderPassResource& renderPass)
{
	if (renderPass.IsValid() == True)
	{
		vkDestroyRenderPass(
			_logicalDevice.device,
			(VkRenderPass)renderPass.GetInstance(),
			nullptr
		);

		renderPass.Invalidate();
	}
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

	std::vector<const char*> extensions = {};
	extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = (uint32_t)queueCreateInfos.size();
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.enabledExtensionCount = extensions.size();
	createInfo.ppEnabledExtensionNames = extensions.data();

	VkResult result = vkCreateDevice(
		_physicalDevice.device,
		&createInfo,
		nullptr,
		&_logicalDevice.device
	);

	if (result != VK_SUCCESS)
		Print("Error: failed to create Vulkan logical device");

	vkGetDeviceQueue(
		_logicalDevice.device,
		_graphicsQueue.familyIndex,
		_graphicsQueue.queueIndex,
		&_graphicsQueue.queue
	);

	vkGetDeviceQueue(
		_logicalDevice.device,
		_transferQueue.familyIndex,
		_transferQueue.queueIndex,
		&_transferQueue.queue
	);

	vkGetDeviceQueue(
		_logicalDevice.device,
		_computeQueue.familyIndex,
		_computeQueue.queueIndex,
		&_computeQueue.queue
	);

	vkGetDeviceQueue(
		_logicalDevice.device,
		_presentQueue.familyIndex,
		_presentQueue.queueIndex,
		&_presentQueue.queue
	);
}

void triton::BGraphicsBackend2Vulkan::DestroyLogicalDevice()
{
	if (_logicalDevice.device != VK_NULL_HANDLE)
		vkDestroyDevice(_logicalDevice.device, nullptr);
}

void triton::BGraphicsBackend2Vulkan::CreateCommandPoolsAndCommandBuffers()
{
	VkResult result;

	VkCommandPoolCreateInfo graphicsCreateInfo = {};
	graphicsCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	graphicsCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	graphicsCreateInfo.queueFamilyIndex = _graphicsQueue.familyIndex;

	result = vkCreateCommandPool(
		_logicalDevice.device,
		&graphicsCreateInfo,
		nullptr,
		&_graphicsQueue.commandPool
	);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to create graphics command pool");

	VkCommandPoolCreateInfo transferCreateInfo = {};
	transferCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	transferCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	transferCreateInfo.queueFamilyIndex = _transferQueue.familyIndex;

	result = vkCreateCommandPool(
		_logicalDevice.device,
		&transferCreateInfo,
		nullptr,
		&_transferQueue.commandPool
	);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to create transfer command pool");

	VkCommandPoolCreateInfo computeCreateInfo = {};
	computeCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	computeCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	computeCreateInfo.queueFamilyIndex = _computeQueue.familyIndex;

	result = vkCreateCommandPool(
		_logicalDevice.device,
		&computeCreateInfo,
		nullptr,
		&_computeQueue.commandPool
	);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to create compute command pool");

	VkCommandBufferAllocateInfo graphicsAllocInfo = {};
	graphicsAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	graphicsAllocInfo.commandPool = _graphicsQueue.commandPool;
	graphicsAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	graphicsAllocInfo.commandBufferCount = 1;

	result = vkAllocateCommandBuffers(
		_logicalDevice.device,
		&graphicsAllocInfo,
		&_graphicsQueue.commandBuffer
	);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to create graphics command buffer");

	VkCommandBufferAllocateInfo transferAllocInfo = {};
	transferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	transferAllocInfo.commandPool = _transferQueue.commandPool;
	transferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	transferAllocInfo.commandBufferCount = 1;

	result = vkAllocateCommandBuffers(
		_logicalDevice.device,
		&transferAllocInfo,
		&_transferQueue.commandBuffer
	);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to create transfer command buffer");

	VkCommandBufferAllocateInfo computeAllocInfo = {};
	computeAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	computeAllocInfo.commandPool = _computeQueue.commandPool;
	computeAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	computeAllocInfo.commandBufferCount = 1;

	result = vkAllocateCommandBuffers(
		_logicalDevice.device,
		&computeAllocInfo,
		&_computeQueue.commandBuffer
	);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to create compute command buffer");
}

void triton::BGraphicsBackend2Vulkan::DestroyCommandPoolsAndCommandBuffers()
{
	vkFreeCommandBuffers(
		_logicalDevice.device,
		_computeQueue.commandPool,
		1,
		&_computeQueue.commandBuffer
	);

	vkFreeCommandBuffers(
		_logicalDevice.device,
		_transferQueue.commandPool,
		1,
		&_transferQueue.commandBuffer
	);

	vkFreeCommandBuffers(
		_logicalDevice.device,
		_graphicsQueue.commandPool,
		1,
		&_graphicsQueue.commandBuffer
	);

	vkDestroyCommandPool(
		_logicalDevice.device,
		_computeQueue.commandPool,
		nullptr
	);

	vkDestroyCommandPool(
		_logicalDevice.device,
		_transferQueue.commandPool,
		nullptr
	);

	vkDestroyCommandPool(
		_logicalDevice.device,
		_graphicsQueue.commandPool,
		nullptr
	);
}

void triton::BGraphicsBackend2Vulkan::CreateSwapchain(const cVector2& size)
{
	VkResult result;

	VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
	result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
		_physicalDevice.device,
		_surface.surface,
		&surfaceCapabilities
	);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to query surface capabilities");

	uint32_t formatCount = 0;
	std::vector<VkSurfaceFormatKHR> formats = {};

	result = vkGetPhysicalDeviceSurfaceFormatsKHR(
		_physicalDevice.device,
		_surface.surface,
		&formatCount,
		nullptr
	);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to query surface format count");

	if (formatCount != 0)
	{
		formats.resize(formatCount);

		result = vkGetPhysicalDeviceSurfaceFormatsKHR(
			_physicalDevice.device,
			_surface.surface,
			&formatCount,
			formats.data()
		);

		if (result != VK_SUCCESS)
			Print("[Vulkan]: Error: failed to query surface formats");
	}

	uint32_t presentModeCount = 0;
	std::vector<VkPresentModeKHR> presentModes = {};

	result = vkGetPhysicalDeviceSurfacePresentModesKHR(
		_physicalDevice.device,
		_surface.surface,
		&presentModeCount,
		nullptr
	);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to query surface present mode count");

	if (presentModeCount != 0)
	{
		presentModes.resize(presentModeCount);

		result = vkGetPhysicalDeviceSurfacePresentModesKHR(
			_physicalDevice.device,
			_surface.surface,
			&presentModeCount,
			presentModes.data()
		);

		if (result != VK_SUCCESS)
			Print("[Vulkan]: Error: failed to query surface present modes");
	}

	if (formats.empty() || presentModes.empty())
		Print("[Vulkan]: Error: inadequate swapchain support");

	VkSurfaceFormatKHR swapchainSurfaceFormat = ChooseSwapchainSurfaceFormat(formats);

	VkPresentModeKHR swapchainPresentMode = ChooseSwapchainPresentMode(presentModes);

	VkExtent2D swapchainExtent = ChooseSwapchainExtent(surfaceCapabilities, size);

	uint32_t swapchainImageCount = surfaceCapabilities.minImageCount + 1;

	if (surfaceCapabilities.maxImageCount > 0 &&
		swapchainImageCount > surfaceCapabilities.maxImageCount)
		swapchainImageCount = surfaceCapabilities.maxImageCount;

	VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.surface = _surface.surface;
	swapchainCreateInfo.minImageCount = swapchainImageCount;
	swapchainCreateInfo.imageFormat = swapchainSurfaceFormat.format;
	swapchainCreateInfo.imageColorSpace = swapchainSurfaceFormat.colorSpace;
	swapchainCreateInfo.imageExtent = swapchainExtent;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	const uint32_t queueFamilyIndices[] = {
		_graphicsQueue.familyIndex,
		_presentQueue.familyIndex
	};

	if (_graphicsQueue.familyIndex != _presentQueue.familyIndex)
	{
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchainCreateInfo.queueFamilyIndexCount = 2;
		swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchainCreateInfo.queueFamilyIndexCount = 0;
		swapchainCreateInfo.pQueueFamilyIndices = nullptr;
	}

	swapchainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.presentMode = swapchainPresentMode;
	swapchainCreateInfo.clipped = VK_TRUE;
	swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

	result = vkCreateSwapchainKHR(
		_logicalDevice.device,
		&swapchainCreateInfo,
		nullptr,
		&_swapchain.swapchain
	);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to create swapchain");

	_swapchain.size = size;
	_swapchain.format = swapchainSurfaceFormat.format;

	GetSwapchainImages();

	CreateSwapchainRenderTarget();
}

void triton::BGraphicsBackend2Vulkan::DestroySwapchain()
{
	DestroySwapchainRenderTarget();

	for (auto& image : _swapchain.images)
		if (image.view != VK_NULL_HANDLE)
			vkDestroyImageView(
				_logicalDevice.device,
				image.view,
				nullptr
			);

	vkDestroySwapchainKHR(
		_logicalDevice.device,
		_swapchain.swapchain,
		nullptr
	);
}

VkSurfaceFormatKHR triton::BGraphicsBackend2Vulkan::ChooseSwapchainSurfaceFormat(
	const std::vector<VkSurfaceFormatKHR>& formats
)
{
	for (const auto& availableFormat : formats)
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
			availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return availableFormat;

	return formats[0];
}

VkPresentModeKHR triton::BGraphicsBackend2Vulkan::ChooseSwapchainPresentMode(
	const std::vector<VkPresentModeKHR>& presentModes
)
{
	for (const auto& availablePresentMode : presentModes)
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			return availablePresentMode;

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D triton::BGraphicsBackend2Vulkan::ChooseSwapchainExtent(
	VkSurfaceCapabilitiesKHR capabilities,
	const cVector2& size
)
{
	if (capabilities.currentExtent.width != UINT32_MAX)
	{
		return capabilities.currentExtent;
	}
	else
	{
		VkExtent2D actualExtent = {
			(uint32_t)size.GetX(),
			(uint32_t)size.GetY(),
		};

		actualExtent.width = std::clamp(
			actualExtent.width,
			capabilities.minImageExtent.width,
			capabilities.maxImageExtent.width
		);
		actualExtent.height = std::clamp(
			actualExtent.height,
			capabilities.minImageExtent.height,
			capabilities.maxImageExtent.height
		);

		return actualExtent;
	}
}

void triton::BGraphicsBackend2Vulkan::GetSwapchainImages()
{
	VkResult result;

	uint32_t imageCount = 0;

	result = vkGetSwapchainImagesKHR(
		_logicalDevice.device,
		_swapchain.swapchain,
		&imageCount,
		nullptr
	);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to query swapchain image count");

	_swapchain.images.reserve(imageCount);

	std::vector<VkImage> images(imageCount);

	result = vkGetSwapchainImagesKHR(
		_logicalDevice.device,
		_swapchain.swapchain,
		&imageCount,
		images.data()
	);

	for (usize i = 0; i < imageCount; i++)
		_swapchain.images.push_back(
			SImageWithView(
				_swapchain.format,
				images[i],
				VK_NULL_HANDLE
			)
		);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to query swapchain images");

	for (auto& image : _swapchain.images)
	{
		VkImageViewCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

		createInfo.image = image.image;
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = _swapchain.format;

		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		VkResult result = vkCreateImageView(
			_logicalDevice.device,
			&createInfo,
			nullptr,
			&image.view
		);

		if (result != VK_SUCCESS)
			Print("[Vulkan]: Error: failed to create swapchain image views");
	}
}

void triton::BGraphicsBackend2Vulkan::CreateSwapchainRenderTarget()
{
	const usize renderTargetCount = _swapchain.images.size();

	_swapchainRenderTargets.reserve(renderTargetCount);

	for (usize i = 0; i < renderTargetCount; i++)
	{
		CGPUTextureResource colorAttachment = CGPUTextureResource(
			(cpuword)_swapchain.images[i].image,
			(cpuword)_swapchain.images[i].view,
			cVector3(_swapchain.size.GetX(), _swapchain.size.GetY(), 0.0f),
			ETextureDimension::Texture2D,
			ETextureFormat::BGRA8_SRGB,
			0
		);

		const std::vector<CGPUTextureResource> colorAttachmentVec = { colorAttachment };

		void* renderTargetInstance = CObjectAllocator::Allocate(sizeof(cpuword), 64);

		_swapchainRenderTargets.push_back(CGPURenderTargetResource(
			(cpuword)renderTargetInstance,
			0,
			colorAttachmentVec,
			CGPUTextureResource::Invalid()
		));
	}
}

void triton::BGraphicsBackend2Vulkan::DestroySwapchainRenderTarget()
{
	for (auto& renderTarget : _swapchainRenderTargets)
	{
		CObjectAllocator::Deallocate((void*)renderTarget.GetInstance());

		renderTarget.Invalidate();
	}
}

void triton::BGraphicsBackend2Vulkan::CreateSwapchainRenderPass()
{
}

void triton::BGraphicsBackend2Vulkan::DestroySwapchainRenderPass()
{
}

VkFormat triton::BGraphicsBackend2Vulkan::TextureFormatToNative(ETextureFormat textureFormat)
{
	if (textureFormat == ETextureFormat::R8)
		return VK_FORMAT_R8_UNORM;
	else if (textureFormat == ETextureFormat::RGBA8)
		return VK_FORMAT_R8G8B8A8_UNORM;
	else if (textureFormat == ETextureFormat::RGBA8_SRGB)
		return VK_FORMAT_R8G8B8A8_SRGB;
	else if (textureFormat == ETextureFormat::BGRA8_SRGB)
		return VK_FORMAT_B8G8R8A8_SRGB;

	return VK_FORMAT_UNDEFINED;
}

VkImageLayout triton::BGraphicsBackend2Vulkan::AttachmentLayoutToNative(EGraphicsImageLayout attachmentLayout)
{
	if (attachmentLayout == EGraphicsImageLayout::ColorAttachment)
		return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	else if (attachmentLayout == EGraphicsImageLayout::DepthStencilAttachment)
		return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	else if (attachmentLayout == EGraphicsImageLayout::Present)
		return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	return VK_IMAGE_LAYOUT_UNDEFINED;
}