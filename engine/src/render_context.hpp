#pragma once

#include <vector>
#include <string>
#include "../../thirdparty/glm/glm/glm.hpp"
#include "types.hpp"
#include "render_manager.hpp"

namespace realware
{
    namespace core
    {
        class cCamera;
    }

    namespace render
    {
        struct sGPUResource
        {
            core::usize Instance;
            core::usize ViewInstance;
        };

        struct sBuffer : public sGPUResource
        {
            enum eType
            {
                VERTEX = 0,
                INDEX = 1,
                UNIFORM = 2,
                LARGE = 3
            };

            core::usize ByteSize;
            eType Type;
            core::s32 Slot;
        };

        struct sVertexArray : public sGPUResource
        {
        };

        struct sShader : public sGPUResource
        {
        };

        struct sTexture : public sGPUResource
        {
            enum eType
            {
                TEXTURE_2D = 0,
                TEXTURE_2D_ARRAY = 1
            };

            enum eFormat
            {
                R8 = 0,
                R8F = 1,
                RGBA8 = 2,
                RGB16F = 3,
                RGBA16F = 4,
                DEPTH_STENCIL = 5
            };

            core::s32 Width;
            core::s32 Height;
            core::s32 Depth;
            eType Type;
            eFormat Format;
            core::s32 Slot = 0;
        };

        struct sRenderTarget : public sGPUResource
        {
            std::vector<sTexture*> ColorAttachments;
            sTexture* DepthAttachment;
        };

        struct sDepthMode
        {
            core::boolean UseDepthTest = core::K_TRUE;
            core::boolean UseDepthWrite = core::K_TRUE;
        };

        struct sBlendMode
        {
            enum eFactor
            {
                ZERO = 0,
                ONE = 1,
                SRC_COLOR = 2,
                INV_SRC_COLOR = 3,
                SRC_ALPHA = 4,
                INV_SRC_ALPHA = 5
            };

            core::usize FactorCount = 0;
            eFactor SrcFactors[8] = { eFactor::ZERO };
            eFactor DstFactors[8] = { eFactor::ZERO };
        };

        struct sRenderPass
        {
            struct sDescriptor
            {
                sVertexArray* VertexArray = nullptr;
                sVertexBufferGeometry::eFormat InputVertexFormat;
                std::vector<sBuffer*> InputBuffers = {};
                std::vector<sTexture*> InputTextures = {};
                std::vector<std::string> InputTextureNames;
                sShader* Shader = nullptr;
                sRenderTarget* RenderTarget = nullptr;
                sDepthMode DepthMode;
                sBlendMode BlendMode;
                glm::vec4 Viewport = glm::vec4(0.0f);
            };

            sDescriptor Desc;
        };

        class cRenderContext
        {

        public:
            cRenderContext() {}
            ~cRenderContext() {}

            virtual sBuffer* CreateBuffer(core::usize byteSize, const sBuffer::eType& type, const void* data) = 0;
            virtual void BindBuffer(const sBuffer* buffer) = 0;
            virtual void UnbindBuffer(const sBuffer* buffer) = 0;
            virtual void WriteBuffer(sBuffer* buffer, core::usize offset, core::usize byteSize, const void* data) = 0;
            virtual void DeleteBuffer(sBuffer* buffer) = 0;
            virtual sVertexArray* CreateVertexArray() = 0;
            virtual void BindVertexArray(const sVertexArray* vertexArray) = 0;
            virtual void BindDefaultVertexArray(const std::vector<sBuffer*>& buffersToBind) = 0;
            virtual void UnbindVertexArray() = 0;
            virtual void DeleteVertexArray(sVertexArray* vertexArray) = 0;
            virtual void BindShader(const sShader* shader) = 0;
            virtual sShader* BindOpaqueShader() = 0;
            virtual sShader* BindTransparentShader() = 0;
            virtual sShader* BindQuadShader() = 0;
            virtual sShader* BindTextShader() = 0;
            virtual sShader* BindWidgetShader() = 0;
            virtual sShader* BindCompositeTransparentShader() = 0;
            virtual void SetShaderUniform(const sShader* shader, const char* name, const glm::mat4& matrix) = 0;
            virtual void SetShaderUniform(const sShader* shader, const char* name, core::usize count, float* values) = 0;
            virtual sTexture* CreateTexture(core::s32 width, core::s32 height, core::s32 depth, const sTexture::eType& type, const sTexture::eFormat& format, const void* data) = 0;
            virtual sTexture* ResizeTexture(sTexture* texture, const glm::vec2& size) = 0;
            virtual void BindTexture(const sShader* shader, const char* name, const sTexture* texture) = 0;
            virtual void UnbindTexture(const sTexture* texture) = 0;
            virtual void WriteTexture(sTexture* texture, const glm::vec3& offset, const glm::vec2& size, const void* data) = 0;
            virtual void WriteTextureToFile(const sTexture* texture, const char* filename) = 0;
            virtual void DeleteTexture(sTexture* texture) = 0;
            virtual sRenderTarget* CreateRenderTarget(const std::vector<sTexture*>& colorAttachments, const sTexture* depthAttachment) = 0;
            virtual void ResizeRenderTargetColors(sRenderTarget* renderTarget, const glm::vec2& size) = 0;
            virtual void ResizeRenderTargetDepth(sRenderTarget* renderTarget, const glm::vec2& size) = 0;
            virtual void UpdateRenderTargetBuffers(sRenderTarget* renderTarget) = 0;
            virtual void BindRenderTarget(const sRenderTarget* renderTarget) = 0;
            virtual void UnbindRenderTarget() = 0;
            virtual void DeleteRenderTarget(sRenderTarget* renderTarget) = 0;
            virtual sRenderPass* CreateRenderPass(const sRenderPass::sDescriptor& descriptor) = 0;
            virtual void BindRenderPass(const sRenderPass* renderPass) = 0;
            virtual void UnbindRenderPass(const sRenderPass* renderPass) = 0;
            virtual void DeleteRenderPass(sRenderPass* renderPass) = 0;
            virtual void BindDefaultInputLayout() = 0;
            virtual void BindDepthMode(const sDepthMode& blendMode) = 0;
            virtual void BindBlendMode(const sBlendMode& blendMode) = 0;
            virtual void Viewport(const glm::vec4& viewport) = 0;
            virtual void ClearColor(core::usize bufferIndex, const glm::vec4& color) = 0;
            virtual void ClearDepth(float depth) = 0;
            virtual void Draw(core::usize indexCount, core::usize vertexOffset, core::usize indexOffset, core::usize instanceCount) = 0;
            virtual void DrawQuad() = 0;
            virtual void DrawQuads(core::usize count) = 0;

        };

        class cOpenGLRenderContext : public cRenderContext
        {

        public:
            cOpenGLRenderContext();
            ~cOpenGLRenderContext();

            virtual sBuffer* CreateBuffer(core::usize byteSize, const sBuffer::eType& type, const void* data) override final;
            virtual void BindBuffer(const sBuffer* buffer) override final;
            virtual void UnbindBuffer(const sBuffer* buffer) override final;
            virtual void WriteBuffer(sBuffer* buffer, core::usize offset, core::usize byteSize, const void* data) override final;
            virtual void DeleteBuffer(sBuffer* buffer) override final;
            virtual sVertexArray* CreateVertexArray() override final;
            virtual void BindVertexArray(const sVertexArray* vertexArray) override final;
            virtual void BindDefaultVertexArray(const std::vector<sBuffer*>& buffersToBind) override final;
            virtual void UnbindVertexArray() override final;
            virtual void DeleteVertexArray(sVertexArray* vertexArray) override final;
            virtual void BindShader(const sShader* shader) override final;
            virtual sShader* BindOpaqueShader() override final;
            virtual sShader* BindTransparentShader() override final;
            virtual sShader* BindQuadShader() override final;
            virtual sShader* BindTextShader() override final;
            virtual sShader* BindWidgetShader() override final;
            virtual sShader* BindCompositeTransparentShader() override final;
            virtual void SetShaderUniform(const sShader* shader, const char* name, const glm::mat4& matrix) override final;
            virtual void SetShaderUniform(const sShader* shader, const char* name, core::usize count, float* values) override final;
            virtual sTexture* CreateTexture(core::s32 width, core::s32 height, core::s32 depth, const sTexture::eType& type, const sTexture::eFormat& format, const void* data) override final;
            virtual sTexture* ResizeTexture(sTexture* texture, const glm::vec2& size) override final;
            virtual void BindTexture(const sShader* shader, const char* name, const sTexture* texture) override final;
            virtual void UnbindTexture(const sTexture* texture) override final;
            virtual void WriteTexture(sTexture* texture, const glm::vec3& offset, const glm::vec2& size, const void* data) override final;
            virtual void WriteTextureToFile(const sTexture* texture, const char* filename) override final;
            virtual void DeleteTexture(sTexture* texture) override final;
            virtual sRenderTarget* CreateRenderTarget(const std::vector<sTexture*>& colorAttachments, const sTexture* depthAttachment) override final;
            virtual void ResizeRenderTargetColors(sRenderTarget* renderTarget, const glm::vec2& size) override final;
            virtual void ResizeRenderTargetDepth(sRenderTarget* renderTarget, const glm::vec2& size) override final;
            virtual void UpdateRenderTargetBuffers(sRenderTarget* renderTarget) override final;
            virtual void BindRenderTarget(const sRenderTarget* renderTarget) override final;
            virtual void UnbindRenderTarget() override final;
            virtual void DeleteRenderTarget(sRenderTarget* renderTarget) override final;
            virtual sRenderPass* CreateRenderPass(const sRenderPass::sDescriptor& descriptor) override final;
            virtual void BindRenderPass(const sRenderPass* renderPass) override final;
            virtual void UnbindRenderPass(const sRenderPass* renderPass) override final;
            virtual void DeleteRenderPass(sRenderPass* renderPass) override final;
            virtual void BindDefaultInputLayout() override final;
            virtual void BindDepthMode(const sDepthMode& blendMode) override final;
            virtual void BindBlendMode(const sBlendMode& blendMode) override final;
            virtual void Viewport(const glm::vec4& viewport) override final;
            virtual void ClearColor(core::usize bufferIndex, const glm::vec4& color) override final;
            virtual void ClearDepth(float depth) override final;
            virtual void Draw(core::usize indexCount, core::usize vertexOffset, core::usize indexOffset, core::usize instanceCount) override final;
            virtual void DrawQuad() override final;
            virtual void DrawQuads(core::usize count) override final;

        };
    }
}