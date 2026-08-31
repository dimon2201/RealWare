// graphics_backend2_vulkan.hpp

#pragma once

#include <vector>
#include <vulkan/vulkan.h>
#include "object.hpp"
#include "graphics_backend2.hpp"
#include "graphics_backend2_vulkan_structs.hpp"
#include "texture_formats.hpp"
#include "render_native_command_struct.hpp"

namespace triton
{
	class BGraphicsBackend2Vulkan : public IGraphicsBackend2
	{
		TRITON_CLASS_NAME(BGraphicsBackend2Vulkan)

		SInstance										_instance;
		SSurface										_surface;
		SPhysicalDevice									_physicalDevice = SPhysicalDevice(
			EGraphicsDeviceType::Unknown, VK_NULL_HANDLE, {}, {}, {}, {}, std::vector<VkQueueFamilyProperties>(), {}
		);
		VkFormat										_physicalDeviceDepthBufferFormat = VK_FORMAT_UNDEFINED;
		SQueue											_graphicsQueue = SQueue(0, 0, {});
		SQueue											_transferQueue = SQueue(0, 0, {});
		SQueue											_computeQueue = SQueue(0, 0, {});
		SQueue											_presentQueue = SQueue(0, 0, {});
		SQueue											_graphicsTransferPresentQueue = SQueue(0, 0, {});
		SLogicalDevice									_logicalDevice;
		SSwapchain										_swapchain;
		std::vector<CGPURenderTargetResource>			_swapchainRenderTargets;
		std::vector<CGPURenderPassResource>				_swapchainRenderPasses;
		CGPUShaderResource								_swapchainShader = CGPUShaderResource::Invalid();
		std::vector<CGPUPipelineResource>				_swapchainPipelines;
		CGPUBindingGroupLayoutResource					_swapchainBindingGroupLayout = CGPUBindingGroupLayoutResource::Invalid();
		std::vector<CGPUBindingGroupResource>			_swapchainBindingGroups;
		std::vector<SSemaphore>							_swapchainImageAvailableSemaphores;
		std::vector<SSemaphore>							_swapchainRenderFinishedSemaphores;
		std::vector<SFence>								_swapchainFences;
		CGPUTextureResource								_swapchainPresentTexture = CGPUTextureResource::Invalid();
		VkDescriptorPool								_descriptorPool = VK_NULL_HANDLE;
		std::vector<VkCommandBuffer>					_commandBuffers;
		types::usize									_framesInFlight = 0;
		types::usize									_currentFrame = 0;
		std::vector<SNativeRenderCommand>				_deferredCommands;

	public:
		explicit BGraphicsBackend2Vulkan(cContext* context) : IGraphicsBackend2(context) {}
		~BGraphicsBackend2Vulkan() override = default;

		// Initialization/Shutdown
		void Initialize(
			SWindowBackend& window,
			types::boolean bEnableDebugging,
			const std::vector<const char*> extensions,
			EGraphicsDeviceType deviceType,
			const cVector2& swapchainSize,
			types::usize framesInFlight
		) override final;

		void Shutdown() override final;

		void FinalizeSwapchain(const CGPUTextureResource& presentTexture) override final;

		void ReleaseSwapchainResources() override final;

		CGPUTextureResource CreateTexture(
			types::boolean bCreateSampler,
			ETextureFormat format,
			types::dword usageMask,
			ETextureDimension dimension,
			const cVector3& size
		) override final;

		void WriteTexture(
			const CGPUTextureResource& texture,
			const cVector3& offset,
			const types::u8* data,
			types::usize byteSize
		) override final;

		void DestroyTexture(CGPUTextureResource& texture) override final;

		CGPUBufferResource CreateBuffer(
			EGPUBufferType type,
			const types::usize byteSize
		) override final;

		void WriteBuffer(
			const CGPUBufferResource& buffer,
			types::usize offset,
			const types::u8* data,
			types::usize byteSize
		) override final;

		void DestroyBuffer(CGPUBufferResource& buffer) override final;

		CGPURenderTargetResource CreateRenderTarget(
			const std::vector<CGPUTextureResource>& colorAttachments,
			const CGPUTextureResource& depthAttachment
		) override final;

		void DestroyRenderTarget(CGPURenderTargetResource& renderTarget) override final;

		CGPUPipelineResource CreatePipeline(
			const CGPUShaderResource& shader,
			const SViewport& viewport,
			CGPURenderTargetResource& renderTarget,
			const CGPURenderPassResource& renderPass,
			const std::vector<CGPUBindingGroupLayoutResource>& bindingGroupLayouts,
			EPrimitiveTopology primitiveTopology,
			EVertexBufferFormat vertexBufferFormat,
			types::boolean bUsePushConstants
		) override final;

		void DestroyPipeline(CGPUPipelineResource& pipeline) override final;

		CGPUBindingGroupLayoutResource CreateBindingGroupLayout(
			const std::vector<SBindingGroupBinding>& bindings
		) override final;

		void DestroyBindingGroupLayout(CGPUBindingGroupLayoutResource& bindingGroupLayout) override final;

		CGPUBindingGroupResource CreateBindingGroup(
			const CGPUBindingGroupLayoutResource& bindingGroupLayout,
			const std::vector<SBindingGroupBinding>& buffersToBind,
			const std::vector<SBindingGroupBinding>& texturesToBind
		) override final;

		void DestroyBindingGroup(CGPUBindingGroupResource& bindingGroup) override final;

		CGPURenderPassResource CreateRenderPass(
			CGPURenderTargetResource& renderTarget,
			types::boolean bClearRenderTarget,
			const cVector4& clearColor,
			types::f32 clearDepth,
			const std::vector<EResourceUsage>& srcAttachmentsUsage,
			const std::vector<EResourceUsage>& dstAttachmentsUsage
		) override final;

		void DestroyRenderPass(CGPURenderPassResource& renderPass) override final;

		CGPUShaderResource CreateShader(
			types::dword stageMask,
			const SShaderBytecodeFiles& bytecodeFiles
		) override final;

		void DestroyShader(CGPUShaderResource& shader) override final;

		void ResetCommandBuffer() override final;

		void AddCommandToBuffer(
			ENativeRenderCommand command,
			const void* commandArgA,
			const void* commandArgB
		) override final;

		void BeginFrame() override final;

		void EndFrame() override final;
		
	private:
		void CreateInstance(types::boolean bEnableDebugging, const std::vector<const char*> extensions);

		void DestroyInstance();

		void CreateDebugMessenger();

		void DestroyDebugMessenger();

		void CreateSurface(SWindowBackend& window);

		void DestroySurface();

		void PickPhysicalDevice(EGraphicsDeviceType deviceType);

		void CheckPhysicalDeviceProperties();

		void CheckPhysicalDeviceFeatures();

		void CheckPhysicalDeviceFeaturesVulkan13();

		void CheckPhysicalDeviceFormats();

		void FindQueueFamilies();

		void CreateLogicalDevice();

		void DestroyLogicalDevice();

		void CreateCommandPoolsAndCommandBuffers();

		void DestroyCommandPoolsAndCommandBuffers();

		void CreateSwapchain(const cVector2& size, types::usize framesInFlight);

		void DestroySwapchain();

		void CreateDescriptorPool();

		void DestroyDescriptorPool();

		VkSurfaceFormatKHR ChooseSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);

		VkPresentModeKHR ChooseSwapchainPresentMode(const std::vector<VkPresentModeKHR>& presentModes);

		VkExtent2D ChooseSwapchainExtent(
			VkSurfaceCapabilitiesKHR capabilities,
			const cVector2& size
		);

		void GetSwapchainImages();

		void CreateSwapchainRenderTargets();

		void DestroySwapchainRenderTargets();

		void CreateSwapchainRenderPasses();

		void DestroySwapchainRenderPasses();

		void CreateSwapchainShader();

		void DestroySwapchainShader();

		void CreateSwapchainSemaphoresAndFences();

		void DestroySwapchainSemaphoresAndFences();

		types::usize FindProperMemoryTypeIndex(
			VkPhysicalDeviceMemoryProperties memoryProperties,
			VkMemoryRequirements requirements,
			types::dword requiredMemoryPropertyBits
		);
		
		types::usize TextureFormatToChannelCount(ETextureFormat textureFormat);

		VkDeviceMemory AllocateDeviceMemory(VkMemoryRequirements requirements, types::dword requiredMemoryPropertyBits);

		VkFormat TextureFormatToNative(ETextureFormat textureFormat);

		VkImageUsageFlags TextureUsageToNative(types::dword textureUsageMask);

		VkImageType TextureDimensionToNative(ETextureDimension textureDimension);

		VkImageLayout AttachmentUsageToNativeLayout(EResourceUsage attachmentUsage);

		VkPipelineBindPoint PipelineBindPointToNative(EPipelineBindPoint bindPoint);

		VkViewport ViewportToNative(const SViewport& viewport);

		VkPrimitiveTopology PrimitiveTopologyToNative(EPrimitiveTopology primitiveTopology);

		VkBufferUsageFlags BufferTypeToNative(EGPUBufferType bufferType);

		std::string ShaderSourceInclude(const std::string& shaderSource, const std::string& includeStr);
	};
}