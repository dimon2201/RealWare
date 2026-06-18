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

    class CVertexArray : public cGPUResource
    {
        TRITON_OBJECT(CVertexArray)

    public:
        explicit CVertexArray(cContext* context, types::qword instance);
        virtual ~CVertexArray() override = default;
    };

    class SShaderDefine final
    {
    public:
        SShaderDefine(const std::string& name, types::usize index);

        std::string name = "";
        types::usize index = 0;
    };

    class CGPUShader : public cGPUResource
    {
        TRITON_OBJECT(CGPUShader)

    public:
        explicit CGPUShader(cContext* context, types::usize instance, types::usize viewInstance) : cGPUResource(context, instance, viewInstance) {}
        ~CGPUShader() override = default;
    };

    class XShader : public iObject
    {
        TRITON_OBJECT(XShader)

        CGPUShader _gpuShader = CGPUShader(nullptr, 0, 0);

    public:
        explicit XShader(cContext* context, const std::string& vertexStr, const std::string& fragmentStr, const std::string& vertexCustomFunc, const std::string& fragmentCustomFunc, const std::vector<SShaderDefine>&& defines);
        ~XShader() override;
    };

    class CRenderTarget : public cGPUResource
    {
        TRITON_OBJECT(CRenderTarget)

        mutable std::vector<cTexture*> _colorAttachments = {};
        cTexture* _depthAttachment = nullptr;

    public:
        explicit CRenderTarget(
            cContext* context,
            types::qword instance,
            const std::vector<cTexture*>& colorAttachments,
            cTexture* depthAttachment
        );
        ~CRenderTarget() override = default;

        inline std::vector<cTexture*>& GetColorAttachments() const { return _colorAttachments; }
        inline cTexture* GetDepthAttachment() const { return _depthAttachment; }
        inline void SetColorAttachments(
            const std::vector<cTexture*>& newColorAttachments
        ) {
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

        virtual void BindShader(const cShader* shader) = 0;
        virtual void UnbindShader() = 0;
        virtual CGPUShader CreateShader(
            EBuiltinRenderPassType builtinType,
            const std::string& vertexStr,
            const std::string& fragmentStr,
            const std::string& vertexCustomFuncStr,
            const std::string& fragmentCustomFuncStr,
            const std::vector<SShaderDefine>& defines = {}
        ) = 0;
        virtual void DestroyShader(const CGPUShader& shader) = 0;
        virtual void SetShaderUniform(const cShader* shader, const std::string& name, const glm::mat4& matrix) = 0;
        virtual void SetShaderUniform(
            const cShader* shader,
            const std::string& name,
            types::usize count,
            const types::f32* values
        ) = 0;
        virtual void BindTextureNamed(
            cShader* shader,
            cTexture* texture,
            const std::string& textureName,
            types::u32 slot
        ) = 0;
        virtual cVertexArray* CreateVertexArray() = 0;
        virtual void BindVertexArray(const cVertexArray* vertexArray) = 0;
        virtual void BindDefaultVertexArray(const std::vector<cBuffer*>& buffersToBind) = 0;
        virtual void UnbindVertexArray() = 0;
        virtual void DestroyVertexArray(cVertexArray* vertexArray) = 0;
        virtual CGPURenderPass CreateRenderPass() = 0;
        virtual void BindRenderPass(const XRenderPass* renderPass, cShader* customShader = nullptr) = 0;
        virtual void UnbindRenderPass(const XRenderPass* renderPass) = 0;
        virtual void DestroyRenderPass(XRenderPassGPU* renderPass) = 0;
        virtual void BindDefaultInputLayout() = 0;
        virtual void BindDepthMode(const sDepthMode& blendMode) = 0;
        virtual void BindBlendMode(const sBlendMode& blendMode) = 0;
        virtual void Viewport(const sViewport& viewport) = 0;
        virtual cRenderTarget* CreateRenderTarget(
            const std::vector<cTexture*>& colorAttachments,
            cTexture* depthAttachment
        ) = 0;
        virtual void ResizeRenderTargetColors(cRenderTarget* renderTarget, const glm::vec2& size) = 0;
        virtual void ResizeRenderTargetDepth(cRenderTarget* renderTarget, const glm::vec2& size) = 0;
        virtual void UpdateRenderTargetBuffers(cRenderTarget*& renderTarget) = 0;
        virtual void BindRenderTarget(const cRenderTarget* renderTarget) = 0;
        virtual void UnbindRenderTarget() = 0;
        virtual void DestroyRenderTarget(cRenderTarget* renderTarget) = 0;
    };
}