// render_context.hpp

#pragma once

#include "graphics_resource_backend.hpp"

namespace triton
{
    /*class cContext;

    class cGraphicsOGLBackend final : public iGraphicsBackend
    {
        TRITON_OBJECT(cGraphicsOGLBackend)

    public:
        explicit cGraphicsOGLBackend(cContext* context, iGraphicsBufferBackend* bufferBackend);
        virtual ~cGraphicsOGLBackend() override final = default;

        virtual void BindWindowContext(void* nativeWindow) override final;
        virtual void CreateGraphicsContext() override final;
        virtual cVertexArray* CreateVertexArray() override final;
        virtual void BindVertexArray(const cVertexArray* vertexArray) override final;
        virtual void BindDefaultVertexArray(const std::vector<cBuffer*>& buffersToBind) override final;
        virtual void UnbindVertexArray() override final;
        virtual void DestroyVertexArray(cVertexArray* vertexArray) override final;
        virtual void BindShader(const cShader* shader) override final;
        virtual void UnbindShader() override final;
        virtual cShader* CreateShader(eCategory renderPath, const std::string& vertexPath, const std::string& fragmentPath, const std::vector<cShader::sDefinePair>& definePairs = {}) override final;
        virtual cShader* CreateShader(const cShader* baseShader, const std::string& vertexFunc, const std::string& fragmentFunc, const std::vector<cShader::sDefinePair>& definePairs = {}) override final;
        virtual void DefineInShader(cShader* shader, const std::vector<cShader::sDefinePair>& definePairs) override final;
        virtual void DestroyShader(cShader* shader) override final;
        virtual void SetShaderUniform(const cShader* shader, const std::string& name, const glm::mat4& matrix) override final;
        virtual void SetShaderUniform(const cShader* shader, const std::string& name, types::usize count, const types::f32* values) override final;
        virtual cTexture* CreateTexture(types::usize width, types::usize height, types::usize depth, cTexture::eDimension dimension, cTexture::eFormat format, const void* data) override final;
        virtual cTexture* ResizeTexture(cTexture* texture, const glm::vec2& size) override final;
        virtual void BindTexture(const cShader* shader, const std::string& name, const cTexture* texture, types::s32 slot) override final;
        virtual void UnbindTexture(const cTexture* texture) override final;
        virtual void WriteTexture(const cTexture* texture, const glm::vec3& offset, const glm::vec2& size, const void* data) override final;
        virtual void WriteTextureToFile(const cTexture* texture, const std::string& filename) override final;
        virtual void GenerateTextureMips(const cTexture* texture) override final;
        virtual void DestroyTexture(cTexture* texture) override final;
        virtual cRenderTarget* CreateRenderTarget(const std::vector<cTexture*>& colorAttachments, cTexture* depthAttachment) override final;
        virtual void ResizeRenderTargetColors(cRenderTarget* renderTarget, const glm::vec2& size) override final;
        virtual void ResizeRenderTargetDepth(cRenderTarget* renderTarget, const glm::vec2& size) override final;
        virtual void UpdateRenderTargetBuffers(cRenderTarget* renderTarget) override final;
        virtual void BindRenderTarget(const cRenderTarget* renderTarget) override final;
        virtual void UnbindRenderTarget() override final;
        virtual void DestroyRenderTarget(cRenderTarget* renderTarget) override final;
        virtual cRenderPassGPU* CreateRenderPass(const sRenderPassDescriptor& desc) override final;
        virtual void BindRenderPass(const cRenderPass* renderPass, cShader* customShader = nullptr) override final;
        virtual void UnbindRenderPass(const cRenderPass* renderPass) override final;
        virtual void DestroyRenderPass(cRenderPassGPU* renderPass) override final;
        virtual void BindDefaultInputLayout() override final;
        virtual void BindDepthMode(const sDepthMode& blendMode) override final;
        virtual void BindBlendMode(const sBlendMode& blendMode) override final;
        virtual void Viewport(const sViewport& viewport) override final;
        virtual void ClearColor(const glm::vec4& color) override final;
        virtual void ClearDepth(types::f32 depth) override final;
        virtual void ClearFramebufferColor(types::usize bufferIndex, const glm::vec4& color) override final;
        virtual void ClearFramebufferDepth(types::f32 depth) override final;
        virtual void Draw(types::usize indexCount, types::usize vertexOffset, types::usize indexOffset, types::usize instanceCount) override final;
        virtual void DrawQuad() override final;
        virtual void DrawQuads(types::usize count) override final;
    };*/
}