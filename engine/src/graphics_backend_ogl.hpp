// graphics_backend_ogl.hpp

#pragma once

#include "graphics_backend.hpp"

namespace triton
{
	class XGraphicsBackendOGL : public IGraphicsBackend
	{
	public:
		explicit XGraphicsBackendOGL(cContext* context) : IGraphicsBackend(context) {}
		~XGraphicsBackendOGL() override = default;

		// Context
		virtual void CreateGraphicsContext(sInputBackendWindow& window) override final;

		virtual void MakeWindowGraphicsContextCurrent(const sInputBackendWindow& window) override final;

		virtual void SwapWindowBuffers(const sInputBackendWindow& window) override final;

		// Drawcall
		virtual void ClearColor(const cVector4& color) override final;

		virtual void ClearDepth(types::f32 depth) override final;

		virtual void ClearFramebufferColor(types::usize bufferIndex, const cVector4& color) override final;

		virtual void ClearFramebufferDepth(types::f32 depth) override final;

		virtual void Draw(
			types::usize indexCount,
			types::usize vertexElementOffset,
			types::usize indexElementOffset,
			types::usize instanceCount
		) override final;

		virtual void DrawQuad() override final;

		virtual void DrawQuads(types::usize count) override final;

        // Pipeline
        virtual CGPUShader CreateShader(
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
        ) override final;

        virtual void BindShader(const CGPUShader& shader) override final;

        virtual void UnbindShader() override final;

        virtual void DestroyShader(const CGPUShader& shader) override final;

        virtual void SetShaderUniform(
            const CGPUShader& shader,
            const std::string& name,
            const glm::mat4& matrix
        ) override final;

        virtual void SetShaderUniform(
            const CGPUShader& shader,
            const std::string& name,
            types::usize count,
            const types::f32* values
        ) override final;

        virtual void SetShaderUniform(
            const CGPUShader& shader,
            const std::string& name,
            types::u32 value
        ) override final;

        virtual void SetShaderUniform(
            const CGPUShader& shader,
            const std::string& name,
            types::s32 value
        ) override final;

        virtual void BindTextureNamed(
            const CGPUShader& shader,
            const CGPUTexture& texture,
            const std::string& textureName,
            types::s32 slot
        ) override final;

        virtual CGPUInputLayout CreateInputLayout() override final;

        virtual void BindInputLayout(const CGPUInputLayout& inputLayout) override final;

        virtual void BindStaticInputLayout() override final;

        virtual void BindSkinnedInputLayout() override final;

        virtual void UnbindInputLayout() override final;

        virtual void DestroyInputLayout(const CGPUInputLayout& vertexArray) override final;

        virtual CGPURenderPass CreateRenderPass() override final;

        virtual void BindRenderPass(const CGPURenderPass& renderPass) override final;

        virtual void UnbindRenderPass(const CGPURenderPass& renderPass) override final;

        virtual void BindDepthState(const SDepthState& blendMode) override final;

        virtual void BindBlendState(const SBlendState& blendMode) override final;

        virtual void SetViewport(const SViewport& viewport) override final;

        virtual CGPURenderTarget CreateRenderTarget(
            const std::vector<CGPUTexture>& colorAttachments,
            const CGPUTexture& depthAttachment
        ) override final;

        virtual void ResizeRenderTargetColors(
            CGPURenderTarget& renderTarget,
            const glm::vec2& size
        ) override final;

        virtual void ResizeRenderTargetDepth(
            CGPURenderTarget& renderTarget,
            const glm::vec2& size
        ) override final;

        virtual void UpdateRenderTargetBuffers(CGPURenderTarget& renderTarget) override final;

        virtual void BindRenderTarget(const CGPURenderTarget& renderTarget) override final;

        virtual void UnbindRenderTarget() override final;

        virtual void DestroyRenderTarget(const CGPURenderTarget& renderTarget) override final;

        // Resource
        virtual CGPUBuffer CreateBuffer(
            EGPUBufferType type,
            const types::u8* data,
            types::usize byteSize,
            types::s32 slot
        ) override final;

        virtual void BindBuffer(const CGPUBuffer& buffer) override final;

        virtual void BindBufferNotVAO(const CGPUBuffer& buffer) override final;

        virtual void UnbindBuffer(const CGPUBuffer& buffer) override final;

        virtual void WriteBuffer(
            const CGPUBuffer& buffer,
            types::usize offset,
            types::usize byteSize,
            const types::u8* data
        ) override final;

        virtual void DestroyBuffer(const CGPUBuffer& buffer) override final;

        virtual CGPUTexture CreateTexture(
            const cVector3& size,
            ETextureDimension dimension,
            ETextureFormat format,
            const types::u8* data,
            types::s32 slot
        ) override final;

        virtual CGPUTexture ResizeTexture(const CGPUTexture& texture, const cVector2& size) override final;

        virtual void BindTexture(const CGPUTexture& texture) override final;

        virtual void UnbindTexture(const CGPUTexture& texture) override final;

        virtual void WriteTexture(
            const CGPUTexture& texture,
            const cVector3& offset,
            const cVector2& size,
            const types::u8* data
        ) override final;

        virtual void WriteTextureToFile(const CGPUTexture& texture, const std::string& filename) override final;

        virtual void GenerateTextureMips(const CGPUTexture& texture) override final;

        virtual void DestroyTexture(const CGPUTexture& texture) override final;
	};
}