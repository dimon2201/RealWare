// graphics_backend2_vulkan.cpp

#include <cstring>
#include <set>
#include <algorithm>
#include <cstdint>
#include "graphics_backend2_vulkan.hpp"
#include "input_backend.hpp"
#include "context.hpp"
#include "object_allocator.hpp"
#include "filesystem_manager.hpp"
#include "log.hpp"
#include "memory_units.hpp"
#include "render_native_draw_command_info_struct.hpp"
#include "rasterizer_state.hpp"

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

triton::CGPUTextureResource triton::BGraphicsBackend2Vulkan::CreateTexture(
	boolean bCreateSampler,
	ETextureFormat format,
	dword usageMask,
	ETextureDimension dimension,
	const cVector3& size
)
{
	const VkFormat nativeFormat = TextureFormatToNative(format);

	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = TextureDimensionToNative(dimension);
	imageCreateInfo.format = nativeFormat;
	imageCreateInfo.extent = {
		(uint32_t)size.GetX(),
		(uint32_t)size.GetY(),
		1
	};
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage = TextureUsageToNative(usageMask);
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VkImage image = VK_NULL_HANDLE;
	
	VkResult result = vkCreateImage(
		_logicalDevice.device,
		&imageCreateInfo,
		nullptr,
		&image
	);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to create image");

	VkPhysicalDeviceMemoryProperties memoryProperties = {};
	vkGetPhysicalDeviceMemoryProperties(
		_physicalDevice.device,
		&memoryProperties
	);

	VkMemoryRequirements requirements = {};
	vkGetImageMemoryRequirements(
		_logicalDevice.device,
		image,
		&requirements
	);

	usize memoryTypeIndex = FindProperImageMemoryType(
		memoryProperties,
		requirements
	);

	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = requirements.size;
	memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;

	VkDeviceMemory memory = VK_NULL_HANDLE;

	result = vkAllocateMemory(
		_logicalDevice.device,
		&memoryAllocateInfo,
		nullptr,
		&memory
	);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to allocate image memory");

	result = vkBindImageMemory(
		_logicalDevice.device,
		image,
		memory,
		(VkDeviceSize)0
	);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to bind image memory");

	VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_NONE;

	if (usageMask & (dword)ETextureUsageBit::Sampled ||
		usageMask & (dword)ETextureUsageBit::ColorAttachment)
		aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	if (usageMask & (dword)ETextureUsageBit::DepthStencilAttachment)
		aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

	VkImageViewCreateInfo imageViewCreateInfo = {};
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.image = image;
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.format = nativeFormat;
	imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.subresourceRange.aspectMask = aspectMask;
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imageViewCreateInfo.subresourceRange.levelCount = 1;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount = 1;

	VkImageView imageView = VK_NULL_HANDLE;

	vkCreateImageView(
		_logicalDevice.device,
		&imageViewCreateInfo,
		nullptr,
		&imageView
	);

	VkSampler sampler = VK_NULL_HANDLE;
	if (bCreateSampler == True)
	{
		VkSamplerCreateInfo samplerCreateInfo = {};
		samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
		samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
		samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.mipLodBias = 0.0f;
		samplerCreateInfo.anisotropyEnable = VK_FALSE;
		samplerCreateInfo.maxAnisotropy = 1.0f;
		samplerCreateInfo.compareEnable = VK_FALSE;
		samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerCreateInfo.minLod = 0.0f;
		samplerCreateInfo.maxLod = 1.0f;
		samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;

		result = vkCreateSampler(
			_logicalDevice.device,
			&samplerCreateInfo,
			nullptr,
			&sampler
		);

		if (result != VK_SUCCESS)
			Print("[Vulkan]: Error: failed to create sampler");
	}

	return CGPUTextureResource(
		(qword)image,
		(qword)imageView,
		bCreateSampler == True ? (qword)sampler : 0,
		(qword)memory,
		format,
		usageMask,
		dimension,
		size,
		0
	);
}

void triton::BGraphicsBackend2Vulkan::DestroyTexture(CGPUTextureResource& texture)
{
	if (texture.IsValid())
	{
		if (texture.GetSampler())
			vkDestroySampler(
				_logicalDevice.device,
				(VkSampler)texture.GetSampler(),
				nullptr
			);

		vkDestroyImageView(
			_logicalDevice.device,
			(VkImageView)texture.GetView(),
			nullptr
		);

		vkDestroyImage(
			_logicalDevice.device,
			(VkImage)texture.GetInstance(),
			nullptr
		);

		vkFreeMemory(
			_logicalDevice.device,
			(VkDeviceMemory)texture.GetDeviceMemory(),
			nullptr
		);
	}
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

triton::CGPUPipelineResource triton::BGraphicsBackend2Vulkan::CreatePipeline(
	const CGPUShaderResource& shader,
	const SViewport& viewport,
	CGPURenderTargetResource& renderTarget,
	const CGPURenderPassResource& renderPass,
	const std::vector<CGPUTextureResource>& texturesToBind
)
{
	const dword stageMask = shader.GetStageMask();

	const boolean bIsVertexPixel =
		(stageMask & (dword)EShaderStage::Vertex &&
			stageMask & (dword)EShaderStage::Pixel) ? True : False;

	const boolean bHasDepth = renderTarget.GetDepthAttachment().IsValid();

	usize stageCount = 0;
	std::vector<VkPipelineShaderStageCreateInfo> stageCreateInfos;

	if (bIsVertexPixel == True)
	{
		stageCount = 2;
		
		VkPipelineShaderStageCreateInfo vertexStageCreateInfo = {};
		vertexStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertexStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertexStageCreateInfo.module = (VkShaderModule)shader.GetModules()[0];
		vertexStageCreateInfo.pName = "main";

		stageCreateInfos.push_back(vertexStageCreateInfo);

		VkPipelineShaderStageCreateInfo pixelStageCreateInfo = {};
		pixelStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		pixelStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		pixelStageCreateInfo.module = (VkShaderModule)shader.GetModules()[1];
		pixelStageCreateInfo.pName = "main";

		stageCreateInfos.push_back(pixelStageCreateInfo);
	}

	VkPipeline pipeline = VK_NULL_HANDLE;

	if (bIsVertexPixel == True)
	{
		VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {};
		vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputStateCreateInfo.vertexBindingDescriptionCount = 0;
		vertexInputStateCreateInfo.pVertexBindingDescriptions = nullptr;
		vertexInputStateCreateInfo.vertexAttributeDescriptionCount = 0;
		vertexInputStateCreateInfo.pVertexAttributeDescriptions = nullptr;

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = {};
		inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

		VkViewport nativeViewport = ViewportToNative(viewport);

		VkRect2D scissor = {};

		VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
		viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportStateCreateInfo.viewportCount = 1;
		viewportStateCreateInfo.pViewports = &nativeViewport;
		viewportStateCreateInfo.scissorCount = 1;
		viewportStateCreateInfo.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = {};
		rasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
		rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
		rasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
		rasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
		rasterizationStateCreateInfo.depthBiasClamp = 0.0f;
		rasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;
		rasterizationStateCreateInfo.lineWidth = 1.0f;

		VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo = {};
		multisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
		multisampleStateCreateInfo.minSampleShading = 1.0f;
		multisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
		multisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {};
		colorBlendAttachmentState.blendEnable = VK_FALSE;
		colorBlendAttachmentState.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT |
			VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT;

		VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {};
		colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
		colorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
		colorBlendStateCreateInfo.attachmentCount = 1;
		colorBlendStateCreateInfo.pAttachments = &colorBlendAttachmentState;
		colorBlendStateCreateInfo.blendConstants[0] = 0.0f;
		colorBlendStateCreateInfo.blendConstants[1] = 0.0f;
		colorBlendStateCreateInfo.blendConstants[2] = 0.0f;
		colorBlendStateCreateInfo.blendConstants[3] = 0.0f;

		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.setLayoutCount = 0;
		pipelineLayoutCreateInfo.pSetLayouts = nullptr;
		pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
		pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

		VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

		VkResult result = vkCreatePipelineLayout(
			_logicalDevice.device,
			&pipelineLayoutCreateInfo,
			nullptr,
			&pipelineLayout
		);

		if (result != VK_SUCCESS)
			Print("[Vulkan]: Error: failed to create pipeline layout");

		VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};
		graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		graphicsPipelineCreateInfo.stageCount = stageCount;
		graphicsPipelineCreateInfo.pStages = stageCreateInfos.data();
		graphicsPipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
		graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
		graphicsPipelineCreateInfo.pTessellationState = nullptr;
		graphicsPipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
		graphicsPipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
		graphicsPipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;

		VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo = {};
		if (bHasDepth == True)
		{
			depthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
			depthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
			depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
			depthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
			depthStencilStateCreateInfo.minDepthBounds = 0.0f;
			depthStencilStateCreateInfo.maxDepthBounds = 1.0f;
			depthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;
			depthStencilStateCreateInfo.front = {};
			depthStencilStateCreateInfo.back = {};

			graphicsPipelineCreateInfo.pDepthStencilState = &depthStencilStateCreateInfo;
		}

		graphicsPipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
		graphicsPipelineCreateInfo.pDynamicState = nullptr;
		graphicsPipelineCreateInfo.layout = pipelineLayout;
		graphicsPipelineCreateInfo.renderPass = (VkRenderPass)renderPass.GetInstance();
		graphicsPipelineCreateInfo.subpass = 0;
		graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
		graphicsPipelineCreateInfo.basePipelineIndex = -1;

		vkCreateGraphicsPipelines(
			_logicalDevice.device,
			VK_NULL_HANDLE,
			1,
			&graphicsPipelineCreateInfo,
			nullptr,
			&pipeline
		);

		SDescriptorSet nativeDescriptorSet = CreateDescriptorSet(texturesToBind);

		std::vector<qword> descriptorSets;
		if (nativeDescriptorSet.set != VK_NULL_HANDLE)
			descriptorSets.push_back((qword)nativeDescriptorSet.set);

		return CGPUPipelineResource(
			(qword)pipeline,
			0,
			EPipelineBindPoint::Graphics,
			descriptorSets
		);
	}

	return CGPUPipelineResource::Invalid();
}

void triton::BGraphicsBackend2Vulkan::DestroyPipeline(CGPUPipelineResource& pipeline)
{
	if (pipeline.IsValid())
	{
		vkDestroyPipeline(
			_logicalDevice.device,
			(VkPipeline)pipeline.GetInstance(),
			nullptr
		);

		std::vector<SDescriptorSet> nativeDescriptorSets;
		for (auto& descriptorSet : pipeline.GetDescriptorSets())
		{
			SDescriptorSet nativeDescriptorSet;
			nativeDescriptorSet.set = (VkDescriptorSet)descriptorSet;

			nativeDescriptorSets.push_back(nativeDescriptorSet);
		}

		for (auto& descriptorSet: nativeDescriptorSets)
			DestroyDescriptorSet(descriptorSet);

		pipeline.Invalidate();
	}
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
	CGPURenderTargetResource& renderTarget,
	boolean bClearRenderTarget
)
{
	const usize colorAttachmentCount = renderTarget.GetColorAttachmentCount();
	const boolean hasDepth = renderTarget.GetDepthAttachment().IsValid();
	const usize depthAttachmentCount = hasDepth == True ? 1 : 0;

	std::vector<VkAttachmentDescription> attachments(colorAttachmentCount + depthAttachmentCount);
	std::vector<VkAttachmentReference> colorReferences(colorAttachmentCount);
	VkAttachmentReference depthReference = {};

	VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_NONE;
	if (bClearRenderTarget == True)
		loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	else if (bClearRenderTarget == False)
		loadOp = VK_ATTACHMENT_LOAD_OP_NONE;

	for (usize i = 0; i < colorAttachmentCount; ++i)
	{
		attachments[i].format = TextureFormatToNative(
			renderTarget.GetColorAttachments()[i].GetFormat()
		);
		attachments[i].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[i].loadOp = loadOp;
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
		attachments[colorAttachmentCount].loadOp = loadOp;
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
	dependency.srcAccessMask = 0;
	dependency.dstStageMask =
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask =
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	if (hasDepth)
	{
		dependency.dstStageMask |=
			VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask |=
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
		(types::qword)framebuffer,
		bClearRenderTarget
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

triton::CGPUShaderResource triton::BGraphicsBackend2Vulkan::CreateShader(
	dword stageMask,
	const SShaderBytecodeFiles& bytecodeFiles
)
{
	if (!stageMask ||
		(!bytecodeFiles.vertexFilePath.has_value() &&
		!bytecodeFiles.pixelFilePath.has_value() &&
		!bytecodeFiles.tessellationControlFilePath.has_value() &&
		!bytecodeFiles.tessellationEvaluationFilePath.has_value() &&
		!bytecodeFiles.computeFilePath.has_value()))
		return CGPUShaderResource::Invalid();

	if (stageMask & (dword)EShaderStage::Vertex &&
		stageMask & (dword)EShaderStage::Pixel &&
		bytecodeFiles.vertexFilePath.has_value() &&
		bytecodeFiles.pixelFilePath.has_value())
	{
		u8* vertexShaderByteCode = nullptr;
		const usize vertexShaderByteCodeSize = _context->GetSubsystem<CFileSystem>()->BinaryFileToArray(
			bytecodeFiles.vertexFilePath.value().string(),
			vertexShaderByteCode,
			0
		);

		u8* pixelShaderByteCode = nullptr;
		const usize pixelShaderByteCodeSize = _context->GetSubsystem<CFileSystem>()->BinaryFileToArray(
			bytecodeFiles.pixelFilePath.value().string(),
			pixelShaderByteCode,
			0
		);

		VkResult result;

		VkShaderModuleCreateInfo vertexShaderModuleCreateInfo = {};
		vertexShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		vertexShaderModuleCreateInfo.codeSize = vertexShaderByteCodeSize;
		vertexShaderModuleCreateInfo.pCode = (uint32_t*)vertexShaderByteCode;

		VkShaderModule vertexShaderModule = VK_NULL_HANDLE;

		result = vkCreateShaderModule(
			_logicalDevice.device,
			&vertexShaderModuleCreateInfo,
			nullptr,
			&vertexShaderModule
		);

		if (result != VK_SUCCESS)
			Print("[Vulkan]: Error: failed to create vertex shader");

		VkShaderModuleCreateInfo pixelShaderModuleCreateInfo = {};
		pixelShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		pixelShaderModuleCreateInfo.codeSize = pixelShaderByteCodeSize;
		pixelShaderModuleCreateInfo.pCode = (uint32_t*)pixelShaderByteCode;

		VkShaderModule pixelShaderModule = VK_NULL_HANDLE;

		result = vkCreateShaderModule(
			_logicalDevice.device,
			&pixelShaderModuleCreateInfo,
			nullptr,
			&pixelShaderModule
		);

		if (result != VK_SUCCESS)
			Print("[Vulkan]: Error: failed to create pixel shader");

		_context->GetSubsystem<CFileSystem>()->ReleaseBinaryFileArray(vertexShaderByteCode);
		_context->GetSubsystem<CFileSystem>()->ReleaseBinaryFileArray(pixelShaderByteCode);

		std::vector<qword> modules(2);
		modules[0] = (qword)vertexShaderModule;
		modules[1] = (qword)pixelShaderModule;

		return CGPUShaderResource(
			0,
			0,
			stageMask,
			modules
		);
	}

	return CGPUShaderResource::Invalid();
}

void triton::BGraphicsBackend2Vulkan::DestroyShader(CGPUShaderResource& shader)
{
	if (shader.IsValid())
	{
		for (auto& module : shader.GetModules())
			vkDestroyShaderModule(
				_logicalDevice.device,
				(VkShaderModule)module,
				nullptr
			);

		shader.Invalidate();
	}
}

void triton::BGraphicsBackend2Vulkan::ResetCommandBuffer()
{
	vkResetCommandBuffer(
		_graphicsQueue.commandBuffer,
		0
	);
}

void triton::BGraphicsBackend2Vulkan::AddCommandToBuffer(
	ENativeRenderCommand command,
	const void* commandArgA,
	const void* commandArgB
)
{
	if (command == ENativeRenderCommand::Unknown ||
		!commandArgA)
		return;

	if (command == ENativeRenderCommand::BeginRenderPass)
	{
		CGPURenderTargetResource& renderTarget = *(CGPURenderTargetResource*)commandArgA;
		CGPURenderPassResource& renderPass = *(CGPURenderPassResource*)commandArgB;

		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = (VkRenderPass)renderPass.GetInstance();
		renderPassBeginInfo.framebuffer = (VkFramebuffer)renderPass.GetFramebuffer();
		renderPassBeginInfo.renderArea.offset = {
			0,
			0
		};
		renderPassBeginInfo.renderArea.extent.width = (uint32_t)renderTarget.GetSize().GetX();
		renderPassBeginInfo.renderArea.extent.width = (uint32_t)renderTarget.GetSize().GetY();
		
		VkClearValue clearValue = {};
		clearValue.color = {
			1.0f,
			0.0f,
			0.0f,
			1.0f
		};

		if (renderPass.IsRenderTargetClearRequired() == True)
		{
			renderPassBeginInfo.clearValueCount = 1;
			renderPassBeginInfo.pClearValues = &clearValue;
		}

		vkCmdBeginRenderPass(
			_graphicsQueue.commandBuffer,
			&renderPassBeginInfo,
			VK_SUBPASS_CONTENTS_INLINE
		);
	}
	else if (command == ENativeRenderCommand::BindPipeline)
	{
		vkCmdBindPipeline(
			_graphicsQueue.commandBuffer,
			PipelineBindPointToNative(((CGPUPipelineResource*)commandArgA)->GetBindingPoint()),
			(VkPipeline)((CGPUPipelineResource*)commandArgA)->GetInstance()
		);
	}
	else if (command == ENativeRenderCommand::Draw)
	{
		vkCmdDraw(
			_graphicsQueue.commandBuffer,
			((SNativeCommandDrawInfo*)commandArgA)->vertexCount,
			((SNativeCommandDrawInfo*)commandArgA)->instanceCount,
			((SNativeCommandDrawInfo*)commandArgA)->firstVertex,
			((SNativeCommandDrawInfo*)commandArgA)->firstInstance
		);
	}
	else if (command == ENativeRenderCommand::EndRenderPass)
	{
		vkCmdEndRenderPass(_graphicsQueue.commandBuffer);
	}
	else if (command == ENativeRenderCommand::SetScissor)
	{
		vkCmdSetScissor(_graphicsQueue.commandBuffer, 0, 0, nullptr);
	}
	else if (command == ENativeRenderCommand::SetViewport)
	{
		SViewport viewport = *(SViewport*)commandArgA;

		VkViewport nativeViewport = ViewportToNative(viewport);

		vkCmdSetViewport(_graphicsQueue.commandBuffer, 0, 1, &nativeViewport);
	}
}

void triton::BGraphicsBackend2Vulkan::BeginFrame()
{
	vkWaitForFences(
		_logicalDevice.device,
		1,
		&_swapchainFence.fence,
		VK_TRUE,
		UINT64_MAX
	);

	vkResetFences(
		_logicalDevice.device,
		1,
		&_swapchainFence.fence
	);

	ResetCommandBuffer();

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	VkResult result = vkBeginCommandBuffer(
		_graphicsQueue.commandBuffer,
		&beginInfo
	);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to begin command buffer");
}

void triton::BGraphicsBackend2Vulkan::EndFrame()
{
	VkResult result;

	uint32_t imageIndex = 0;

	result = vkAcquireNextImageKHR(
		_logicalDevice.device,
		_swapchain.swapchain,
		UINT64_MAX,
		_swapchainImageAvailableSemaphore.semaphore,
		VK_NULL_HANDLE,
		&imageIndex
	);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to acquire swapchain image");

	VkClearValue clearValue = {};
	clearValue.color = {
		1.0f,
		0.0f,
		0.0f,
		1.0f
	};

	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = (VkRenderPass)_swapchainRenderPasses[imageIndex].GetInstance();
	renderPassBeginInfo.framebuffer = (VkFramebuffer)_swapchainRenderPasses[imageIndex].GetFramebuffer();
	renderPassBeginInfo.renderArea.offset = { 0, 0 };
	renderPassBeginInfo.renderArea.extent.width = _swapchain.size.GetX();
	renderPassBeginInfo.renderArea.extent.height = _swapchain.size.GetY();
	renderPassBeginInfo.clearValueCount = 1;
	renderPassBeginInfo.pClearValues = &clearValue;

	vkCmdBeginRenderPass(
		_graphicsQueue.commandBuffer,
		&renderPassBeginInfo,
		VK_SUBPASS_CONTENTS_INLINE
	);

	vkCmdEndRenderPass(
		_graphicsQueue.commandBuffer
	);

	vkEndCommandBuffer(
		_graphicsQueue.commandBuffer
	);

	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &_swapchainImageAvailableSemaphore.semaphore;
	submitInfo.pWaitDstStageMask = &waitStage;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &_graphicsQueue.commandBuffer;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &_swapchainRenderFinishedSemaphores[imageIndex].semaphore;

	result = vkQueueSubmit(
		_graphicsQueue.queue,
		1,
		&submitInfo,
		_swapchainFence.fence
	);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to submit commands to the graphics queue");

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &_swapchainRenderFinishedSemaphores[imageIndex].semaphore;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &_swapchain.swapchain;
	presentInfo.pImageIndices = &imageIndex;

	result = vkQueuePresentKHR(
		_presentQueue.queue,
		&presentInfo
	);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to present frame");
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
	CheckPhysicalDeviceFormats();
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

void triton::BGraphicsBackend2Vulkan::CheckPhysicalDeviceFormats()
{
	const VkFormat candidates[] =
	{
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D24_UNORM_S8_UINT
	};

	for (VkFormat format : candidates)
	{
		VkFormatProperties properties{};

		vkGetPhysicalDeviceFormatProperties(
			_physicalDevice.device,
			format,
			&properties
		);

		if (properties.optimalTilingFeatures &
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
			_physicalDeviceDepthBufferFormat = format;
	}
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

	CreateSwapchainRenderTargets();

	CreateSwapchainRenderPasses();

	CreateSwapchainSemaphoresAndFence();
}

void triton::BGraphicsBackend2Vulkan::DestroySwapchain()
{
	DestroySwapchainSemaphoresAndFence();

	DestroySwapchainRenderTargets();

	DestroySwapchainRenderPasses();

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

void triton::BGraphicsBackend2Vulkan::CreateSwapchainRenderTargets()
{
	const usize renderTargetCount = _swapchain.images.size();

	_swapchainRenderTargets.reserve(renderTargetCount);

	for (usize i = 0; i < renderTargetCount; i++)
	{
		CGPUTextureResource colorAttachment = CGPUTextureResource(
			(qword)_swapchain.images[i].image,
			(qword)_swapchain.images[i].view,
			0,
			0,
			ETextureFormat::BGRA8_SRGB,
			(dword)ETextureUsageBit::ColorAttachment,
			ETextureDimension::Texture2D,
			cVector3(_swapchain.size.GetX(), _swapchain.size.GetY(), 0.0f),
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

	for (usize i = 0; i < renderTargetCount; i++)
		_swapchainRenderTargets[i].SetColorAttachmentLayout(
			0,
			EGraphicsImageLayout::Undefined,
			EGraphicsImageLayout::Present
		);
}

void triton::BGraphicsBackend2Vulkan::DestroySwapchainRenderTargets()
{
	for (auto& renderTarget : _swapchainRenderTargets)
	{
		CObjectAllocator::Deallocate((void*)renderTarget.GetInstance());

		renderTarget.Invalidate();
	}
}

void triton::BGraphicsBackend2Vulkan::CreateSwapchainRenderPasses()
{
	_swapchainRenderPasses.reserve(_swapchainRenderTargets.size());
	for (auto& renderTarget : _swapchainRenderTargets)
		_swapchainRenderPasses.push_back(CreateRenderPass(renderTarget, True));
}

void triton::BGraphicsBackend2Vulkan::DestroySwapchainRenderPasses()
{
	for (auto& renderPass : _swapchainRenderPasses)
		DestroyRenderPass(renderPass);
}

void triton::BGraphicsBackend2Vulkan::CreateSwapchainSemaphoresAndFence()
{
	VkResult result;

	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	result = vkCreateSemaphore(
		_logicalDevice.device,
		&semaphoreCreateInfo,
		nullptr,
		&_swapchainImageAvailableSemaphore.semaphore
	);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to create swapchain semaphore");

	const usize renderFinishedSemaphoreCount = _swapchain.images.size();
	_swapchainRenderFinishedSemaphores.resize(renderFinishedSemaphoreCount);

	for (usize i = 0; i < renderFinishedSemaphoreCount; i++)
	{
		result = vkCreateSemaphore(
			_logicalDevice.device,
			&semaphoreCreateInfo,
			nullptr,
			&_swapchainRenderFinishedSemaphores[i].semaphore
		);

		if (result != VK_SUCCESS)
			Print("[Vulkan]: Error: failed to create swapchain semaphore");
	}

	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

	result = vkCreateFence(
		_logicalDevice.device,
		&fenceCreateInfo,
		nullptr,
		&_swapchainFence.fence
	);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to create swapchain fence");
}

void triton::BGraphicsBackend2Vulkan::DestroySwapchainSemaphoresAndFence()
{
	for (auto& semaphore : _swapchainRenderFinishedSemaphores)
		vkDestroySemaphore(
			_logicalDevice.device,
			semaphore.semaphore,
			nullptr
		);

	vkDestroySemaphore(
		_logicalDevice.device,
		_swapchainImageAvailableSemaphore.semaphore,
		nullptr
	);
}

triton::SDescriptorSet triton::BGraphicsBackend2Vulkan::CreateDescriptorSet(
	const std::vector<CGPUTextureResource>& texturesToBind
)
{
	const usize textureToBindCount = texturesToBind.size();

	const usize bindingCount = textureToBindCount;

	if (!bindingCount)
		return {};

	std::vector<VkDescriptorSetLayoutBinding> bindings(bindingCount);
	std::vector<VkDescriptorImageInfo> bindingImageInfos(bindingCount);

	usize bindingCounter = 0;

	for (usize i = 0; i < textureToBindCount; ++i, ++bindingCounter)
	{
		const CGPUTextureResource& textureToBind = texturesToBind[i];

		bindings[i].binding = bindingCounter;
		bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		bindings[i].descriptorCount = 1;
		bindings[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorImageInfo descriptorImageInfo = {};
		descriptorImageInfo.sampler = (VkSampler)textureToBind.GetSampler();
		descriptorImageInfo.imageView = (VkImageView)textureToBind.GetView();
		descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		bindingImageInfos[i] = descriptorImageInfo;
	}

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo = {};
	descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutInfo.bindingCount = bindingCount;
	descriptorSetLayoutInfo.pBindings = bindings.data();

	VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;

	VkResult result;

	result = vkCreateDescriptorSetLayout(
		_logicalDevice.device,
		&descriptorSetLayoutInfo,
		nullptr,
		&descriptorSetLayout
	);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to create descriptor set layout");

	VkDescriptorPoolSize poolSize = {};
	poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSize.descriptorCount = bindingCount;

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	poolInfo.maxSets = 1;

	VkDescriptorPool descriptorPool = VK_NULL_HANDLE;

	result = vkCreateDescriptorPool(
		_logicalDevice.device,
		&poolInfo,
		nullptr,
		&descriptorPool
	);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to create descriptor pool");

	VkDescriptorSetAllocateInfo descriptorSetAllocInfo = {};
	descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocInfo.descriptorPool = descriptorPool;
	descriptorSetAllocInfo.descriptorSetCount = 1;
	descriptorSetAllocInfo.pSetLayouts = &descriptorSetLayout;

	VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

	result = vkAllocateDescriptorSets(
		_logicalDevice.device,
		&descriptorSetAllocInfo,
		&descriptorSet
	);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to allocate descriptor set");

	std::vector<VkWriteDescriptorSet> writeDescriptorSetElements(bindingCount);
	for (usize i = 0; i < bindingCount; i++)
	{
		VkWriteDescriptorSet writeDescriptorSet = {};
		writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet.dstSet = descriptorSet;
		writeDescriptorSet.dstBinding = i;
		writeDescriptorSet.dstArrayElement = 0;
		writeDescriptorSet.descriptorType = bindings[i].descriptorType;
		writeDescriptorSet.descriptorCount = 1;
		writeDescriptorSet.pImageInfo = &bindingImageInfos[i];

		writeDescriptorSetElements[i] = writeDescriptorSet;
	}

	vkUpdateDescriptorSets(
		_logicalDevice.device,
		bindingCount,
		writeDescriptorSetElements.data(),
		0,
		nullptr
	);

	SDescriptorSet finalDescriptorSet;
	finalDescriptorSet.pool = descriptorPool;
	finalDescriptorSet.layout = descriptorSetLayout;
	finalDescriptorSet.set = descriptorSet;

	return finalDescriptorSet;
}

void triton::BGraphicsBackend2Vulkan::DestroyDescriptorSet(const SDescriptorSet& descriptorSet)
{
	if (descriptorSet.set != VK_NULL_HANDLE)
		vkFreeDescriptorSets(
			_logicalDevice.device,
			descriptorSet.pool,
			1,
			&descriptorSet.set
		);

	if (descriptorSet.layout != VK_NULL_HANDLE)
		vkDestroyDescriptorSetLayout(
			_logicalDevice.device,
			descriptorSet.layout,
			nullptr
		);

	if (descriptorSet.pool != VK_NULL_HANDLE)
		vkDestroyDescriptorPool(
			_logicalDevice.device,
			descriptorSet.pool,
			nullptr
		);
}

usize triton::BGraphicsBackend2Vulkan::FindProperImageMemoryType(
	VkPhysicalDeviceMemoryProperties memoryProperties,
	VkMemoryRequirements requirements
)
{
	uint32_t memoryTypeIndex = UINT32_MAX;

	for (usize i = 0; i < memoryProperties.memoryTypeCount; ++i)
	{
		const bool typeSupported = (requirements.memoryTypeBits & (1u << i)) != 0;

		const bool propertiesSupported =
			(memoryProperties.memoryTypes[i].propertyFlags &
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			== VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

		if (typeSupported && propertiesSupported)
			return i;
	}

	return 0;
}

VkFormat triton::BGraphicsBackend2Vulkan::TextureFormatToNative(ETextureFormat textureFormat)
{
	if (textureFormat == ETextureFormat::R8)
		return VK_FORMAT_R8_UNORM;
	else if (textureFormat == ETextureFormat::RGBA8)
		return VK_FORMAT_R8G8B8A8_UNORM;
	else if (textureFormat == ETextureFormat::RGBA8_SRGB ||
		textureFormat == ETextureFormat::RGBA8_SRGB_Mips)
		return VK_FORMAT_R8G8B8A8_SRGB;
	else if (textureFormat == ETextureFormat::BGRA8_SRGB)
		return VK_FORMAT_B8G8R8A8_SRGB;
	else if (textureFormat == ETextureFormat::DepthStencil)
		return _physicalDeviceDepthBufferFormat;

	return VK_FORMAT_UNDEFINED;
}

VkImageUsageFlags triton::BGraphicsBackend2Vulkan::TextureUsageToNative(dword textureUsageMask)
{
	VkImageUsageFlags flags = 0;

	if (textureUsageMask & (dword)ETextureUsageBit::Sampled)
		flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
	if (textureUsageMask & (dword)ETextureUsageBit::ColorAttachment)
		flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	if (textureUsageMask & (dword)ETextureUsageBit::DepthStencilAttachment)
		flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

	return flags;
}

VkImageType triton::BGraphicsBackend2Vulkan::TextureDimensionToNative(ETextureDimension textureDimension)
{
	VkImageType type = VK_IMAGE_TYPE_2D;

	return type;
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

VkPipelineBindPoint triton::BGraphicsBackend2Vulkan::PipelineBindPointToNative(EPipelineBindPoint bindPoint)
{
	return VK_PIPELINE_BIND_POINT_GRAPHICS;
}

VkViewport triton::BGraphicsBackend2Vulkan::ViewportToNative(const SViewport& viewport)
{
	VkViewport nativeViewport = {};
	nativeViewport.x = viewport.rect.GetX();
	nativeViewport.y = viewport.rect.GetY();
	nativeViewport.width = viewport.rect.GetZ();
	nativeViewport.height = viewport.rect.GetW();
	nativeViewport.minDepth = 0.0f;
	nativeViewport.maxDepth = 1.0f;

	return nativeViewport;
}

std::string triton::BGraphicsBackend2Vulkan::ShaderSourceInclude(
	const std::string& shaderSource,
	const std::string& includeStr
)
{
	return includeStr + std::string("\n\n") + shaderSource;
}