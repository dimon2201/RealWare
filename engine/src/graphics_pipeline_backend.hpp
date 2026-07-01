// graphics_pipeline_backend.hpp

#pragma once

#include "gpu_resource.hpp"
#include "backend.hpp"
#include "math.hpp"
#include "category.hpp"
#include "render_pass.hpp"
#include "types.hpp"

namespace triton
{
    class cContext;
    class cBuffer;
    class cTexture;
    class cTextureAtlasTexture;
    class XRenderPass;
    class XInstanceBuffer;

    class CGPUVertexArray : public cGPUResource
    {
        TRITON_OBJECT(CGPUVertexArray)

    public:
        explicit CGPUVertexArray(cContext* context, types::qword instance) : cGPUResource(context, instance, 0) {}
        ~CGPUVertexArray() override = default;
    };

    class XVertexArray : public iObject
    {
        TRITON_OBJECT(XVertexArray)

        CGPUVertexArray _gpuVertexArray = CGPUVertexArray(nullptr, 0);

    public:
        explicit XVertexArray(cContext* context, const std::vector<cBuffer*>& buffersToBind);
        ~XVertexArray() override;

        inline const CGPUVertexArray& GetGPUVertexArray() const
        {
            return _gpuVertexArray;
        }
    };

    class CGPUShader : public cGPUResource
    {
        TRITON_OBJECT(CGPUShader)

    public:
        explicit CGPUShader(cContext* context) : cGPUResource(context, 0, 0) {}
        explicit CGPUShader(cContext* context, types::usize instance, types::usize viewInstance) : cGPUResource(context, instance, viewInstance) {}
        ~CGPUShader() override = default;
    };

    class XShader : public iObject
    {
        TRITON_OBJECT(XShader)

        CGPUShader _gpuShader = CGPUShader(nullptr, 0, 0);

    public:
        explicit XShader(cContext* context, const std::string& vertexStr, const std::string& fragmentStr, const std::string& vertexCustomFunc, const std::string& fragmentCustomFunc, std::vector<SShaderDefine>&& defines = {});
        ~XShader() override;

        inline CGPUShader GetGPUShader() const
        {
            return _gpuShader;
        }
    };

    class XRenderTarget : public cGPUResource
    {
        TRITON_OBJECT(XRenderTarget)

        mutable std::vector<cTexture*> _colorAttachments = {};
        cTexture* _depthAttachment = nullptr;

    public:
        explicit XRenderTarget(
            cContext* context,
            types::qword instance,
            const std::vector<cTexture*>& colorAttachments,
            cTexture* depthAttachment
        );
        ~XRenderTarget() override = default;

        inline std::vector<cTexture*>& GetColorAttachments() const { return _colorAttachments; }
        inline cTexture* GetDepthAttachment() const { return _depthAttachment; }
        inline void SetColorAttachments(const std::vector<cTexture*>& newColorAttachments)
        {
            _colorAttachments = newColorAttachments;
        }
        inline void SetDepthAttachment(
            cTexture* newDepthAttachment
        ) {
            _depthAttachment = newDepthAttachment;
        }
    };

    class CGPURenderPass final : public cGPUResource
    {
    public:
        explicit CGPURenderPass(cContext* context, types::qword instance, types::qword viewInstance)
            : cGPUResource(context, instance, viewInstance) {}
        ~CGPURenderPass() override = default;
    };

    class iGraphicsPipelineBackend : public iBackend
    {
        TRITON_OBJECT(iGraphicsPipelineBackend)

    public:
        explicit iGraphicsPipelineBackend(cContext* context);
        virtual ~iGraphicsPipelineBackend() override = default;

        virtual void BindShader(const CGPUShader* shader) = 0;
        virtual void UnbindShader() = 0;
        virtual CGPUShader CreateShader(
            const char* vertexStr,
            const char* fragmentStr,
            const char* vertexCustomFuncStr,
            const char* fragmentCustomFuncStr,
            types::usize defineCount,
            const SShaderDefine* defines
        ) = 0;
        virtual void DestroyShader(const CGPUShader& shader) = 0;
        virtual void SetShaderUniform(const CGPUShader* shader, const std::string& name, const glm::mat4& matrix) = 0;
        virtual void SetShaderUniform(
            const CGPUShader* shader,
            const std::string& name,
            types::usize count,
            const types::f32* values
        ) = 0;
        virtual void SetShaderUniform(
            const CGPUShader* shader,
            const std::string& name,
            types::u32 value
        ) = 0;
        virtual void SetShaderUniform(
            const CGPUShader* shader,
            const std::string& name,
            types::s32 value
        ) = 0;
        virtual void BindTextureNamed(
            CGPUShader* shader,
            cTexture* texture,
            const std::string& textureName,
            types::s32 slot
        ) = 0;
        virtual CGPUVertexArray CreateVertexArray() = 0;
        virtual void BindVertexArray(const CGPUVertexArray& vertexArray) = 0;
        virtual void BindDefaultVertexArray(const std::vector<cBuffer*>& buffersToBind) = 0;
        virtual void UnbindVertexArray() = 0;
        virtual void DestroyVertexArray(const CGPUVertexArray& vertexArray) = 0;
        virtual CGPURenderPass CreateRenderPass() = 0;
        virtual void BindRenderPass(const XRenderPass* renderPass, CGPUShader* customShader = nullptr) = 0;
        virtual void UnbindRenderPass(const XRenderPass* renderPass) = 0;
        virtual void BindDefaultInputLayout() = 0;
        virtual void BindDepthMode(const SDepthState& blendMode) = 0;
        virtual void BindBlendMode(const SBlendState& blendMode) = 0;
        virtual void Viewport(const SViewport& viewport) = 0;
        virtual XRenderTarget* CreateRenderTarget(
            const std::vector<cTexture*>& colorAttachments,
            cTexture* depthAttachment
        ) = 0;
        virtual void ResizeRenderTargetColors(XRenderTarget* renderTarget, const glm::vec2& size) = 0;
        virtual void ResizeRenderTargetDepth(XRenderTarget* renderTarget, const glm::vec2& size) = 0;
        virtual void UpdateRenderTargetBuffers(XRenderTarget*& renderTarget) = 0;
        virtual void BindRenderTarget(const XRenderTarget* renderTarget) = 0;
        virtual void UnbindRenderTarget() = 0;
        virtual void DestroyRenderTarget(XRenderTarget* renderTarget) = 0;
    };
}