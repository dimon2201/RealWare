// graphics_pipeline_backend.hpp

#pragma once

#include "graphics_pipeline_backend.hpp"
#include "types.hpp"

namespace triton
{
    class cContext;

    class cGraphicsPipelineBackendOGL final : public iGraphicsPipelineBackend
    {
        TRITON_OBJECT(cGraphicsPipelineBackendOGL)

    public:
        explicit cGraphicsPipelineBackendOGL(cContext* context);
        virtual ~cGraphicsPipelineBackendOGL() override final = default;

        virtual void BindShader(const cShader* shader) override final;
        virtual void UnbindShader() override final;
        virtual cShader* CreateShader(
            sRenderPassDescriptor::eRenderPath renderPath,
            const std::string& vertexPath,
            const std::string& fragmentPath,
            const std::vector<cShader::sDefinePair>& definePairs = {}
        ) override final;
        virtual cShader* CreateShader(
            const cShader* baseShader,
            const std::string& vertexFunc,
            const std::string& fragmentFunc,
            const std::vector<cShader::sDefinePair>& definePairs = {}
        ) override final;
        virtual void DestroyShader(cShader* shader) override final;
        virtual void SetShaderUniform(const cShader* shader, const std::string& name, const glm::mat4& matrix) override final;
        virtual void SetShaderUniform(
            const cShader* shader,
            const std::string& name,
            types::usize count,
            const types::f32* values
        ) override final;
        virtual void BindTextureNamed(
            cShader* shader,
            cTexture* texture,
            const std::string& textureName,
            types::u32 slot
        ) override final;
        virtual cVertexArray* CreateVertexArray() override final;
        virtual void BindVertexArray(const cVertexArray* vertexArray) override final;
        virtual void BindDefaultVertexArray(const std::vector<cBuffer*>& buffersToBind) override final;
        virtual void UnbindVertexArray() override final;
        virtual void DestroyVertexArray(cVertexArray* vertexArray) override final;
        virtual cRenderPassGPU* CreateRenderPass(const sRenderPassDescriptor& desc) override final;
        virtual void BindRenderPass(const cRenderPass* renderPass, cShader* customShader = nullptr) override final;
        virtual void UnbindRenderPass(const cRenderPass* renderPass) override final;
        virtual void DestroyRenderPass(cRenderPassGPU* renderPass) override final;
        virtual void BindDefaultInputLayout() override final;
        virtual void BindDepthMode(const sDepthMode& blendMode) override final;
        virtual void BindBlendMode(const sBlendMode& blendMode) override final;
        virtual void Viewport(const sViewport& viewport) override final;
        virtual cRenderTarget* CreateRenderTarget(
            const std::vector<cTexture*>& colorAttachments,
            cTexture* depthAttachment
        ) override final;
        virtual void ResizeRenderTargetColors(cRenderTarget* renderTarget, const glm::vec2& size) override final;
        virtual void ResizeRenderTargetDepth(cRenderTarget* renderTarget, const glm::vec2& size) override final;
        virtual void UpdateRenderTargetBuffers(cRenderTarget*& renderTarget) override final;
        virtual void BindRenderTarget(const cRenderTarget* renderTarget) override final;
        virtual void UnbindRenderTarget() override final;
        virtual void DestroyRenderTarget(cRenderTarget* renderTarget) override final;
    };
}