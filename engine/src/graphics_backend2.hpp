// graphics_backend2.hpp

#pragma once

#include <filesystem>
#include <optional>
#include <vector>
#include "backend.hpp"
#include "math.hpp"
#include "input_window_backend.hpp"
#include "graphics_device_type_enum.hpp"
#include "graphics_texture_usage_enum.hpp"
#include "texture_dimensions.hpp"
#include "gpu_texture_resource.hpp"
#include "gpu_render_target_resource.hpp"
#include "gpu_render_pass_resource.hpp"
#include "gpu_shader_resource.hpp"
#include "gpu_pipeline_resource.hpp"
#include "shader_stage_bit_enum.hpp"
#include "shader_bytecode_files_struct.hpp"
#include "render_native_command_enum.hpp"
#include "render_primitive_topology_enum.hpp"
#include "rasterizer_state.hpp"

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
			const cVector2& swapchainSize
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
			const std::vector<CGPUTextureResource>& texturesToBind,
			EPrimitiveTopology primitiveTopology
		) = 0;

		virtual void DestroyPipeline(CGPUPipelineResource& pipeline) = 0;

		virtual CGPURenderPassResource CreateRenderPass(
			CGPURenderTargetResource& renderTarget,
			types::boolean bClearRenderTarget,
			const cVector4& clearColor,
			types::f32 clearDepth
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