// graphics_pipeline_backend.hpp

#pragma once

#include "graphics_pipeline_backend.hpp"
#include "gpu_resource.hpp"
#include "types.hpp"

namespace triton
{
    class cContext;
    class XRenderPass;

    class cGraphicsPipelineBackendOGL final : public iGraphicsPipelineBackend
    {
        TRITON_OBJECT(cGraphicsPipelineBackendOGL)

    public:
        explicit cGraphicsPipelineBackendOGL(cContext* context);
        virtual ~cGraphicsPipelineBackendOGL() override final = default;

        virtual void BindShader(const CGPUShader* shader) override final;
        virtual void UnbindShader() override final;
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
        virtual void DestroyShader(const CGPUShader& shader) override final;
        virtual void SetShaderUniform(const CGPUShader* shader, const std::string& name, const glm::mat4& matrix) override final;
        virtual void SetShaderUniform(
            const CGPUShader* shader,
            const std::string& name,
            types::usize count,
            const types::f32* values
        ) override final;
        virtual void SetShaderUniform(
            const CGPUShader* shader,
            const std::string& name,
            types::u32 value
        ) override final;
        virtual void SetShaderUniform(
            const CGPUShader* shader,
            const std::string& name,
            types::s32 value
        ) override final;
        virtual void BindTextureNamed(
            CGPUShader* shader,
            cTexture* texture,
            const std::string& textureName,
            types::s32 slot
        ) override final;
        virtual CGPUVertexArray CreateVertexArray() override final;
        virtual void BindVertexArray(const CGPUVertexArray& vertexArray) override final;
        virtual void BindDefaultVertexArray(const std::vector<cBuffer*>& buffersToBind) override final;
        virtual void UnbindVertexArray() override final;
        virtual void DestroyVertexArray(const CGPUVertexArray& vertexArray) override final;
        virtual CGPURenderPass CreateRenderPass() override final;
        virtual void BindRenderPass(const XRenderPass* renderPass, CGPUShader* customShader = nullptr) override final;
        virtual void UnbindRenderPass(const XRenderPass* renderPass) override final;
        virtual void BindStaticInputLayout() override final;
        virtual void BindSkinnedInputLayout() override final;
        virtual void BindDepthMode(const SDepthState& blendMode) override final;
        virtual void BindBlendMode(const SBlendState& blendMode) override final;
        virtual void Viewport(const SViewport& viewport) override final;
        virtual XRenderTargetBackend* CreateRenderTarget(
            const std::vector<cTexture*>& colorAttachments,
            cTexture* depthAttachment
        ) override final;
        virtual void ResizeRenderTargetColors(XRenderTargetBackend* renderTarget, const glm::vec2& size) override final;
        virtual void ResizeRenderTargetDepth(XRenderTargetBackend* renderTarget, const glm::vec2& size) override final;
        virtual void UpdateRenderTargetBuffers(XRenderTargetBackend*& renderTarget) override final;
        virtual void BindRenderTarget(const XRenderTargetBackend* renderTarget) override final;
        virtual void UnbindRenderTarget() override final;
        virtual void DestroyRenderTarget(XRenderTargetBackend* renderTarget) override final;
    };
}