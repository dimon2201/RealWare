// graphics_pipeline_backend.hpp

#pragma once

#include "graphics_pipeline_backend.hpp"
#include "render_pass.hpp"
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

        virtual void BindShader(const XShader* shader) override final;
        virtual void UnbindShader() override final;
        virtual CGPUShader CreateShader(
            EBuiltinRenderPassType builtinType,
            const std::string& vertexStr,
            const std::string& fragmentStr,
            const std::string& vertexCustomFuncStr,
            const std::string& fragmentCustomFuncStr,
            const std::vector<SShaderDefine>& defines = {}
        ) override final;
        virtual void DestroyShader(const CGPUShader& shader) override final;
        virtual void SetShaderUniform(const XShader* shader, const std::string& name, const glm::mat4& matrix) override final;
        virtual void SetShaderUniform(
            const XShader* shader,
            const std::string& name,
            types::usize count,
            const types::f32* values
        ) override final;
        virtual void BindTextureNamed(
            XShader* shader,
            cTexture* texture,
            const std::string& textureName,
            types::u32 slot
        ) override final;
        virtual CVertexArray* CreateVertexArray() override final;
        virtual void BindVertexArray(const CVertexArray* vertexArray) override final;
        virtual void BindDefaultVertexArray(const std::vector<cBuffer*>& buffersToBind) override final;
        virtual void UnbindVertexArray() override final;
        virtual void DestroyVertexArray(CVertexArray* vertexArray) override final;
        virtual CGPURenderPass CreateRenderPass() override final;
        virtual void BindRenderPass(const XRenderPass* renderPass, XShader* customShader = nullptr) override final;
        virtual void UnbindRenderPass(const XRenderPass* renderPass) override final;
        virtual void DestroyRenderPass(XRenderPassGPU* renderPass) override final;
        virtual void BindDefaultInputLayout() override final;
        virtual void BindDepthMode(const SDepthState& blendMode) override final;
        virtual void BindBlendMode(const SBlendState& blendMode) override final;
        virtual void Viewport(const sViewport& viewport) override final;
        virtual XRenderTarget* CreateRenderTarget(
            const std::vector<cTexture*>& colorAttachments,
            cTexture* depthAttachment
        ) override final;
        virtual void ResizeRenderTargetColors(XRenderTarget* renderTarget, const glm::vec2& size) override final;
        virtual void ResizeRenderTargetDepth(XRenderTarget* renderTarget, const glm::vec2& size) override final;
        virtual void UpdateRenderTargetBuffers(XRenderTarget*& renderTarget) override final;
        virtual void BindRenderTarget(const XRenderTarget* renderTarget) override final;
        virtual void UnbindRenderTarget() override final;
        virtual void DestroyRenderTarget(XRenderTarget* renderTarget) override final;
    };
}