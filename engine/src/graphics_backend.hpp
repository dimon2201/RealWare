// graphics_backend.hpp

#pragma once

#include "backend.hpp"
#include "input_backend_window.hpp"
#include "math.hpp"
#include "gpu_resource.hpp"
#include "shader_define.hpp"
#include "gpu_texture_resource.hpp"
#include "gpu_shader_resource.hpp"
#include "gpu_buffer.hpp"
#include "gpu_input_layout_resource.hpp"
#include "gpu_render_pass_resource.hpp"
#include "gpu_render_target_resource.hpp"
#include "rasterizer_state.hpp"
#include "gpu_buffer_types.hpp"

namespace triton
{
    class cContext;

	class IGraphicsBackend : public iBackend
	{
	public:
		explicit IGraphicsBackend(cContext* context) : iBackend(context) {}
		~IGraphicsBackend() override = default;

		// Context
		virtual void CreateGraphicsContext(sInputBackendWindow& window) = 0;

		virtual void MakeWindowGraphicsContextCurrent(const sInputBackendWindow& window) = 0;

		virtual void SwapWindowBuffers(const sInputBackendWindow& window) = 0;

		// Drawcall
		virtual void ClearColor(const cVector4& color) = 0;

		virtual void ClearDepth(types::f32 depth) = 0;

		virtual void ClearFramebufferColor(types::usize bufferIndex, const cVector4& color) = 0;

		virtual void ClearFramebufferDepth(types::f32 depth) = 0;

		virtual void Draw(
			types::usize indexCount,
			types::usize vertexElementOffset,
			types::usize indexElementOffset,
			types::usize instanceCount
		) = 0;

		virtual void DrawQuad() = 0;

		virtual void DrawQuads(types::usize count) = 0;

		// Pipeline
        virtual CGPUShaderResource CreateShader(
            const char* vertexStr,
            const char* fragmentStr,
            const char* vertexCustomFuncStr,
            const char* fragmentCustomFuncStr,
            types::usize defineCount,
            const SShaderDefine* defines,
            types::usize vertexIncludePathCount,
            const char** vertexIncludePaths,
            types::usize fragmentIncludePathCount,
            const char** fragmentIncludePaths
        ) = 0;

        virtual void BindShader(const CGPUShaderResource& shader) = 0;

        virtual void UnbindShader() = 0;

        virtual void DestroyShader(const CGPUShaderResource& shader) = 0;

        virtual void SetShaderUniform(
            const CGPUShaderResource& shader,
            const std::string& name,
            const glm::mat4& matrix
        ) = 0;

        virtual void SetShaderUniform(
            const CGPUShaderResource& shader,
            const std::string& name,
            types::usize count,
            const types::f32* values
        ) = 0;

        virtual void SetShaderUniform(
            const CGPUShaderResource& shader,
            const std::string& name,
            types::u32 value
        ) = 0;

        virtual void SetShaderUniform(
            const CGPUShaderResource& shader,
            const std::string& name,
            types::s32 value
        ) = 0;

        virtual void BindTextureNamed(
            const CGPUShaderResource& shader,
            const CGPUTextureResource& texture,
            const std::string& textureName,
            types::s32 slot
        ) = 0;

        virtual CGPUInputLayoutResource CreateInputLayout() = 0;

        virtual void BindInputLayout(const CGPUInputLayoutResource& vertexArray) = 0;

        virtual void BindStaticInputLayout() = 0;

        virtual void BindSkinnedInputLayout() = 0;

        virtual void UnbindInputLayout() = 0;

        virtual void DestroyInputLayout(const CGPUInputLayoutResource& vertexArray) = 0;

        virtual CGPURenderPassResource CreateRenderPass() = 0;

        virtual void BindRenderPass(const CGPURenderPassResource& renderPass) = 0;

        virtual void UnbindRenderPass(const CGPURenderPassResource& renderPass) = 0;

        virtual void BindDepthState(const SDepthState& blendMode) = 0;

        virtual void BindBlendState(const SBlendState& blendMode) = 0;

        virtual void SetViewport(const SViewport& viewport) = 0;

        virtual CGPURenderTargetResource CreateRenderTarget(
            const std::vector<CGPUTextureResource>& colorAttachments,
            const CGPUTextureResource& depthAttachment
        ) = 0;

        virtual void ResizeRenderTargetColors(
            CGPURenderTargetResource& renderTarget,
            const glm::vec2& size
        ) = 0;

        virtual void ResizeRenderTargetDepth(
            CGPURenderTargetResource& renderTarget,
            const glm::vec2& size
        ) = 0;

        virtual void UpdateRenderTargetBuffers(CGPURenderTargetResource& renderTarget) = 0;

        virtual void BindRenderTarget(const CGPURenderTargetResource& renderTarget) = 0;

        virtual void UnbindRenderTarget() = 0;

        virtual void DestroyRenderTarget(const CGPURenderTargetResource& renderTarget) = 0;

        // Resource
        virtual CGPUBufferResource CreateBuffer(
            EGPUBufferType type,
            const types::u8* data,
            types::usize byteSize,
            types::s32 slot
        ) = 0;

        virtual void BindBuffer(const CGPUBufferResource& buffer) = 0;

        virtual void BindBufferNotVAO(const CGPUBufferResource& buffer) = 0;

        virtual void UnbindBuffer(const CGPUBufferResource& buffer) = 0;

        virtual void WriteBuffer(
            const CGPUBufferResource& buffer,
            types::usize offset,
            types::usize byteSize,
            const types::u8* data
        ) = 0;

        virtual void DestroyBuffer(const CGPUBufferResource& buffer) = 0;

        virtual CGPUTextureResource CreateTexture(
            const cVector3& size,
            ETextureDimension dimension,
            ETextureFormat format,
            const types::u8* data,
            types::s32 slot
        ) = 0;

        virtual CGPUTextureResource ResizeTexture(const CGPUTextureResource& texture, const cVector2& size) = 0;

        virtual void BindTexture(const CGPUTextureResource& texture) = 0;

        virtual void UnbindTexture(const CGPUTextureResource& texture) = 0;

        virtual void WriteTexture(
            const CGPUTextureResource& texture,
            const cVector3& offset,
            const cVector2& size,
            const types::u8* data
        ) = 0;

        virtual void WriteTextureToFile(const CGPUTextureResource& texture, const std::string& filename) = 0;

        virtual void GenerateTextureMips(const CGPUTextureResource& texture) = 0;

        virtual void DestroyTexture(const CGPUTextureResource& texture) = 0;
	};
}