// graphics_backend2.hpp

#pragma once

#include <filesystem>
#include <optional>
#include <vector>
#include "backend.hpp"
#include "math.hpp"
#include "input_window_backend.hpp"
#include "gpu_buffer_types.hpp"
#include "graphics_device_type_enum.hpp"
#include "graphics_texture_usage_enum.hpp"
#include "texture_dimensions.hpp"
#include "gpu_texture_resource.hpp"
#include "gpu_buffer_resource.hpp"
#include "gpu_render_target_resource.hpp"
#include "gpu_render_pass_resource.hpp"
#include "gpu_shader_resource.hpp"
#include "gpu_pipeline_resource.hpp"
#include "gpu_binding_group_resource.hpp"
#include "gpu_binding_group_layout_resource.hpp"
#include "shader_stage_bit_enum.hpp"
#include "shader_bytecode_files_struct.hpp"
#include "render_native_command_enum.hpp"
#include "render_primitive_topology_enum.hpp"
#include "render_resource_usage_enum.hpp"
#include "render_pass_push_constants_struct.hpp"
#include "render_binding_group_binding_struct.hpp"
#include "rasterizer_state.hpp"
#include "vertex_buffer_format.hpp"

namespace triton
{
	class cContext;

	class IGraphicsBackend2 : public iBackend
	{
        TRITON_CLASS_NAME(IGraphicsBackend2)

	public:
        explicit IGraphicsBackend2(cContext* context) : iBackend(context) {}
		~IGraphicsBackend2() override = default;

		// Initialization/Shutdown
		virtual void Initialize(
			SWindowBackend& window,
			types::boolean bEnableDebugging,
			const std::vector<const char*> extensions,
			EGraphicsDeviceType deviceType,
			const cVector2& swapchainSize,
			types::usize framesInFlight
		) = 0;

		virtual void Shutdown() = 0;

		virtual void FinalizeSwapchain(const CGPUTextureResource& presentTexture) = 0;

		virtual void ReleaseSwapchainResources() = 0;

		virtual CGPUTextureResource CreateTexture(
			types::boolean bCreateSampler,
			ETextureFormat format,
			types::dword usageMask,
			ETextureDimension dimension,
			const cVector3& size
		) = 0;

		virtual void DestroyTexture(CGPUTextureResource& texture) = 0;

		virtual CGPUBufferResource CreateBuffer(
			EGPUBufferType type,
			const types::usize byteSize
		) = 0;

		virtual void WriteBuffer(
			const CGPUBufferResource& buffer,
			types::usize offset,
			const types::u8* data,
			types::usize byteSize
		) = 0;
		
		virtual void DestroyBuffer(CGPUBufferResource& buffer) = 0;

		virtual CGPURenderTargetResource CreateRenderTarget(
			const std::vector<CGPUTextureResource>& colorAttachments,
			const CGPUTextureResource& depthAttachment
		) = 0;

		virtual void DestroyRenderTarget(CGPURenderTargetResource& renderTarget) = 0;

		virtual CGPUPipelineResource CreatePipeline(
			const CGPUShaderResource& shader,
			const SViewport& viewport,
			CGPURenderTargetResource& renderTarget,
			const CGPURenderPassResource& renderPass,
			const std::vector<CGPUBindingGroupLayoutResource>& bindingGroupLayouts,
			EPrimitiveTopology primitiveTopology,
			EVertexBufferFormat vertexBufferFormat,
			types::boolean bUsePushConstants
		) = 0;

		virtual void DestroyPipeline(CGPUPipelineResource& pipeline) = 0;

		virtual CGPUBindingGroupLayoutResource CreateBindingGroupLayout(
			const std::vector<SBindingGroupBinding>& bindings
		) = 0;

		virtual void DestroyBindingGroupLayout(CGPUBindingGroupLayoutResource& bindingGroupLayout) = 0;

		virtual CGPUBindingGroupResource CreateBindingGroup(
			const CGPUBindingGroupLayoutResource& bindingGroupLayout,
			const std::vector<SBindingGroupBinding>& buffersToBind,
			const std::vector<SBindingGroupBinding>& texturesToBind
		) = 0;

		virtual void DestroyBindingGroup(CGPUBindingGroupResource& bindingGroup) = 0;

		virtual CGPURenderPassResource CreateRenderPass(
			CGPURenderTargetResource& renderTarget,
			types::boolean bClearRenderTarget,
			const cVector4& clearColor,
			types::f32 clearDepth,
			const std::vector<EResourceUsage>& srcAttachmentsUsage,
			const std::vector<EResourceUsage>& dstAttachmentsUsage
		) = 0;

		virtual void DestroyRenderPass(CGPURenderPassResource& renderPass) = 0;

		virtual CGPUShaderResource CreateShader(
			types::dword stageMask,
			const SShaderBytecodeFiles& bytecodeFiles
		) = 0;

		virtual void DestroyShader(CGPUShaderResource& shader) = 0;

		virtual void ResetCommandBuffer() = 0;

		virtual void AddCommandToBuffer(
			ENativeRenderCommand command,
			const void* commandArgA,
			const void* commandArgB
		) = 0;

		virtual void BeginFrame() = 0;

		virtual void EndFrame() = 0;
	};
}