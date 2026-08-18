// graphics_backend.hpp

#pragma once

#include "backend.hpp"
#include "input_backend_window.hpp"
#include "math.hpp"
#include "gpu_resource.hpp"
#include "shader_define.hpp"
#include "gpu_texture.hpp"
#include "rasterizer_state.hpp"
#include "gpu_buffer_types.hpp"

namespace triton
{
    class cContext;

    class CGPUShader : public cGPUResource
    {
        TRITON_OBJECT(CGPUShader)

    public:
        explicit CGPUShader() = default;
        explicit CGPUShader(
            cContext* context,
            types::qword instance,
            types::qword viewInstance
        ) : cGPUResource(context, instance, viewInstance) {}
        ~CGPUShader() override = default;
    };

    class CGPUInputLayout : public cGPUResource
    {
        TRITON_OBJECT(CGPUInputLayout)

    public:
        explicit CGPUInputLayout() = default;
        explicit CGPUInputLayout(
            cContext* context,
            types::qword instance,
            types::qword viewInstance
        ) : cGPUResource(context, instance, viewInstance) {}
        ~CGPUInputLayout() override = default;
    };

    class CGPURenderPass : public cGPUResource
    {
        TRITON_OBJECT(CGPURenderPass)

    public:
        explicit CGPURenderPass() = default;
        explicit CGPURenderPass(
            cContext* context,
            types::qword instance,
            types::qword viewInstance
        ) : cGPUResource(context, instance, viewInstance) {}
        ~CGPURenderPass() override = default;
    };

    class CGPURenderTarget : public cGPUResource
    {
        TRITON_OBJECT(CGPURenderTarget)

        types::usize _colorAttachmentCount = 0;
        CGPUTexture _colorAttachments[8];
        CGPUTexture _depthAttachment = CGPUTexture(
            nullptr,
            0,
            0,
            cVector3(0.0f),
            ETextureDimension::Unknown,
            ETextureFormat::Unknown,
            -1
        );

    public:
        explicit CGPURenderTarget() = default;
        explicit CGPURenderTarget(
            cContext* context,
            types::qword instance,
            types::qword viewInstance,
            types::usize colorAttachmentCount,
            const CGPUTexture* colorAttachments,
            const CGPUTexture& depthAttachment
        ) : cGPUResource(context, instance, viewInstance),
            _depthAttachment(depthAttachment)
        {
            SetColorAttachments(colorAttachmentCount, colorAttachments);
        }
        ~CGPURenderTarget() override = default;

        inline types::usize GetColorAttachmentCount() const { return _colorAttachmentCount; }

        inline CGPUTexture* GetColorAttachments() { return _colorAttachments; }

        inline CGPUTexture& GetDepthAttachment() { return _depthAttachment; }

        inline void SetColorAttachments(
            types::usize colorAttachmentCount,
            const CGPUTexture* colorAttachments
        )
        {
            _colorAttachmentCount = colorAttachmentCount;
            for (types::usize i = 0; i < _colorAttachmentCount; i++)
                _colorAttachments[i] = colorAttachments[i];
        }

        inline void SetDepthAttachment(const CGPUTexture& depthAttachment) { _depthAttachment = depthAttachment; }
    };

    class CGPUBuffer : public cGPUResource
    {
        TRITON_OBJECT(CGPUBuffer)

        EGPUBufferType _type = EGPUBufferType::Unknown;
        types::usize _byteSize = 0;
        types::s32 _slot = -1;

    public:
        explicit CGPUBuffer() = default;
        explicit CGPUBuffer(
            cContext* context,
            types::qword instance,
            types::qword viewInstance,
            EGPUBufferType type,
            types::usize byteSize,
            types::s32 slot
        ) : cGPUResource(context, instance, viewInstance), _type(type), _byteSize(byteSize), _slot(slot) {}
        ~CGPUBuffer() override = default;

        inline EGPUBufferType GetBufferType() const { return _type; }

        inline types::usize GetByteSize() const { return _byteSize; }

        inline types::s32 GetSlot() const { return _slot; }
    };

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
        ) = 0;

        virtual void BindShader(const CGPUShader& shader) = 0;

        virtual void UnbindShader() = 0;

        virtual void DestroyShader(const CGPUShader& shader) = 0;

        virtual void SetShaderUniform(
            const CGPUShader& shader,
            const std::string& name,
            const glm::mat4& matrix
        ) = 0;

        virtual void SetShaderUniform(
            const CGPUShader& shader,
            const std::string& name,
            types::usize count,
            const types::f32* values
        ) = 0;

        virtual void SetShaderUniform(
            const CGPUShader& shader,
            const std::string& name,
            types::u32 value
        ) = 0;

        virtual void SetShaderUniform(
            const CGPUShader& shader,
            const std::string& name,
            types::s32 value
        ) = 0;

        virtual void BindTextureNamed(
            const CGPUShader& shader,
            const CGPUTexture& texture,
            const std::string& textureName,
            types::s32 slot
        ) = 0;

        virtual CGPUInputLayout CreateInputLayout() = 0;

        virtual void BindInputLayout(const CGPUInputLayout& vertexArray) = 0;

        virtual void BindStaticInputLayout() = 0;

        virtual void BindSkinnedInputLayout() = 0;

        virtual void UnbindInputLayout() = 0;

        virtual void DestroyInputLayout(const CGPUInputLayout& vertexArray) = 0;

        virtual CGPURenderPass CreateRenderPass() = 0;

        virtual void BindRenderPass(const CGPURenderPass& renderPass) = 0;

        virtual void UnbindRenderPass(const CGPURenderPass& renderPass) = 0;

        virtual void BindDepthState(const SDepthState& blendMode) = 0;

        virtual void BindBlendState(const SBlendState& blendMode) = 0;

        virtual void SetViewport(const SViewport& viewport) = 0;

        virtual CGPURenderTarget CreateRenderTarget(
            const std::vector<CGPUTexture>& colorAttachments,
            const CGPUTexture& depthAttachment
        ) = 0;

        virtual void ResizeRenderTargetColors(
            CGPURenderTarget& renderTarget,
            const glm::vec2& size
        ) = 0;

        virtual void ResizeRenderTargetDepth(
            CGPURenderTarget& renderTarget,
            const glm::vec2& size
        ) = 0;

        virtual void UpdateRenderTargetBuffers(CGPURenderTarget& renderTarget) = 0;

        virtual void BindRenderTarget(const CGPURenderTarget& renderTarget) = 0;

        virtual void UnbindRenderTarget() = 0;

        virtual void DestroyRenderTarget(const CGPURenderTarget& renderTarget) = 0;

        // Resource
        virtual CGPUBuffer CreateBuffer(
            EGPUBufferType type,
            const types::u8* data,
            types::usize byteSize,
            types::s32 slot
        ) = 0;

        virtual void BindBuffer(const CGPUBuffer& buffer) = 0;

        virtual void BindBufferNotVAO(const CGPUBuffer& buffer) = 0;

        virtual void UnbindBuffer(const CGPUBuffer& buffer) = 0;

        virtual void WriteBuffer(
            const CGPUBuffer& buffer,
            types::usize offset,
            types::usize byteSize,
            const types::u8* data
        ) = 0;

        virtual void DestroyBuffer(const CGPUBuffer& buffer) = 0;

        virtual CGPUTexture CreateTexture(
            const cVector3& size,
            ETextureDimension dimension,
            ETextureFormat format,
            const types::u8* data,
            types::s32 slot
        ) = 0;

        virtual CGPUTexture ResizeTexture(const CGPUTexture& texture, const cVector2& size) = 0;

        virtual void BindTexture(const CGPUTexture& texture) = 0;

        virtual void UnbindTexture(const CGPUTexture& texture) = 0;

        virtual void WriteTexture(
            const CGPUTexture& texture,
            const cVector3& offset,
            const cVector2& size,
            const types::u8* data
        ) = 0;

        virtual void WriteTextureToFile(const CGPUTexture& texture, const std::string& filename) = 0;

        virtual void GenerateTextureMips(const CGPUTexture& texture) = 0;

        virtual void DestroyTexture(const CGPUTexture& texture) = 0;
	};
}