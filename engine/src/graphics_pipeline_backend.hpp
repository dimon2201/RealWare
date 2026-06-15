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

    class cVertexArray : public cGPUResource
    {
        TRITON_OBJECT(cVertexArray)

    public:
        explicit cVertexArray(cContext* context, types::qword instance);
        virtual ~cVertexArray() override = default;
    };

    class cShader : public cGPUResource
    {
        TRITON_OBJECT(cShader)

        std::string _vertex = "";
        std::string _fragment = "";

    public:
        struct sDefinePair
        {
            sDefinePair(const std::string& name, types::usize index);
            ~sDefinePair() = default;

            std::string name = "";
            types::usize index = 0;
        };

        explicit cShader(
            cContext* context,
            types::qword instance,
            const std::string& vertexShaderStr,
            const std::string& fragmentShaderStr
        );
        virtual ~cShader() override = default;

        inline const std::string& GetVertexStr() const { return _vertex; }
        inline const std::string& GetFragmentStr() const { return _fragment; }
    };

    class cRenderTarget : public cGPUResource
    {
        TRITON_OBJECT(cRenderTarget)

        mutable std::vector<cTexture*> _colorAttachments = {};
        cTexture* _depthAttachment = nullptr;

    public:
        explicit cRenderTarget(
            cContext* context,
            types::qword instance,
            const std::vector<cTexture*>& colorAttachments,
            cTexture* depthAttachment
        );
        virtual ~cRenderTarget() override = default;

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

    class XRenderPassGPU : public iObject
    {
        TRITON_OBJECT(XRenderPassGPU)

        cVertexArray* _vertexArray = nullptr;
        cShader* _shader = nullptr;
        XInstanceBuffer* _instanceBuffer = nullptr;
        cBuffer* _materialBuffer = nullptr;
        cBuffer* _textureBuffer = nullptr;

    public:
        explicit XRenderPassGPU(cContext* context, cVertexArray* vertexArray, cShader* shader, XInstanceBuffer* instanceBuffer, cBuffer* materialBuffer, cBuffer* textureBuffer);
        virtual ~XRenderPassGPU() override final = default;

        inline cVertexArray* GetVertexArray() const { return _vertexArray; }
        inline cShader* GetShader() const { return _shader; }
        
        inline XInstanceBuffer* GetInstanceBuffer() const
        {
            return _instanceBuffer;
        }

        inline cBuffer* GetMaterialBuffer() const
        {
            return _materialBuffer;
        }

        inline cBuffer* GetTextureBuffer() const
        {
            return _textureBuffer;
        }
    };

    class iGraphicsPipelineBackend : public iBackend
    {
        TRITON_OBJECT(iGraphicsPipelineBackend)

    public:
        explicit iGraphicsPipelineBackend(cContext* context);
        virtual ~iGraphicsPipelineBackend() override = default;

        virtual void BindShader(const cShader* shader) = 0;
        virtual void UnbindShader() = 0;
        virtual cShader* CreateShader(
            SRenderPassDescriptor::eRenderPath renderPath,
            const std::string& vertexPath,
            const std::string& fragmentPath,
            const std::vector<cShader::sDefinePair>& definePairs = {}
        ) = 0;
        virtual cShader* CreateShader(
            const cShader* baseShader,
            const std::string& vertexFunc,
            const std::string& fragmentFunc,
            const std::vector<cShader::sDefinePair>& definePairs = {}
        ) = 0;
        virtual void DestroyShader(cShader* shader) = 0;
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
        virtual XRenderPassGPU* CreateRenderPass(const SRenderPassDescriptor& desc) = 0;
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