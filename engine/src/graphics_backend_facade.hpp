// graphics_backend_facade.hpp

#pragma once

#include <vector>
#include <string>
#include "../../thirdparty/glm/glm/glm.hpp"
#include "category.hpp"
#include "object.hpp"
#include "backend.hpp"
#include "math.hpp"
#include "gpu_resource.hpp"
#include "graphics_buffer_backend.hpp"
#include "types.hpp"

namespace triton
{
    struct sInputBackendWindow;
    class cApplication;
    class cTextureAtlasTexture;
    class cRenderPass;
    class cRenderTarget;
    class cTexture;

    class cVertexArray : public cGPUResource
    {
        TRITON_OBJECT(cVertexArray)

        friend class cGraphicsOGLBackend;

    public:
        explicit cVertexArray(cContext* context);
    };

    class cShader : public cGPUResource
    {
        TRITON_OBJECT(cShader)

        friend class cGraphicsOGLBackend;

        std::string _vertex = "";
        std::string _fragment = "";

    public:
        struct sDefinePair
        {
            sDefinePair(const std::string& name, types::usize index) : _name(name), _index(index) {}
            ~sDefinePair() = default;

            std::string _name = "";
            types::usize _index = 0;
        };

        explicit cShader(cContext* context);
    };

    class cRenderTarget : public cGPUResource
    {
        TRITON_OBJECT(cRenderTarget)

        friend class cGraphicsOGLBackend;

        mutable std::vector<cTexture*> _colorAttachments = {};
        cTexture* _depthAttachment = nullptr;

        inline std::vector<cTexture*>& GetColorAttachments() const { return _colorAttachments; }
        inline cTexture* GetDepthAttachment() const { return _depthAttachment; }
        inline void SetDepthAttachment(cTexture* attachment) { _depthAttachment = attachment; }

    public:
        explicit cRenderTarget(cContext* context);
    };

    struct sDepthMode
    {
        types::boolean useDepthTest = types::K_TRUE;
        types::boolean useDepthWrite = types::K_TRUE;
    };

    struct sBlendMode
    {
        enum class eFactor
        {
            ZERO = 0,
            ONE = 1,
            SRC_COLOR = 2,
            INV_SRC_COLOR = 3,
            SRC_ALPHA = 4,
            INV_SRC_ALPHA = 5
        };

        types::usize factorCount = 0;
        eFactor srcFactors[8] = { eFactor::ZERO };
        eFactor dstFactors[8] = { eFactor::ZERO };
    };

    struct sViewport
    {
        cVector4 rect = cVector4(0.0f);
    };

    struct sRenderPassDescriptor
    {
        eCategory inputVertexFormat = eCategory::VERTEX_BUFFER_FORMAT_NONE;
        std::vector<cBuffer*> inputBuffers = {};
        std::vector<cTexture*> inputTextures = {};
        std::vector<std::string> inputTextureNames = {};
        std::vector<cTextureAtlasTexture*> inputTextureAtlasTextures = {};
        std::vector<std::string> inputTextureAtlasTextureNames = {};
        eCategory shaderRenderPath = eCategory::RENDER_PATH_OPAQUE;
        std::string shaderVertexPath = "";
        std::string shaderFragmentPath = "";
        std::string shaderVertexFunc = "";
        std::string shaderFragmentFunc = "";
        cShader* shaderBase = nullptr;
        sDepthMode depthMode = {};
        sBlendMode blendMode = {};
        sViewport viewport = {};
        cRenderTarget* renderTarget = nullptr;
    };

    class cRenderPassGPU : public iObject
    {
        TRITON_OBJECT(cRenderPassGPU)

        friend class cGraphicsOGLBackend;

        cVertexArray* _vertexArray = nullptr;
        cShader* _shader = nullptr;

    public:
        explicit cRenderPassGPU(cContext* context, cVertexArray* vertexArray, cShader* shader);
        virtual ~cRenderPassGPU() override final = default;
        
        inline cVertexArray* GetVertexArray() const { return _vertexArray; }
        inline cShader* GetShader() const { return _shader; }
    };

    class cGraphicsBackendFacade : public iObject
    {
        TRITON_OBJECT(cGraphicsBackendFacade)

        iGraphicsBufferBackend* _buffer = nullptr;

    public:
        explicit cGraphicsBackendFacade(cContext* context, iGraphicsBufferBackend* bufferBackend);
        virtual ~cGraphicsBackendFacade() override = default;

        inline iGraphicsBufferBackend* GetBufferBackend() const { return _buffer; }

        /*virtual void BindWindowContext(void* nativeWindow) = 0;
        virtual void CreateGraphicsContext() = 0;
        virtual cVertexArray* CreateVertexArray() = 0;
        virtual void BindVertexArray(const cVertexArray* vertexArray) = 0;
        virtual void BindDefaultVertexArray(const std::vector<cBuffer*>& buffersToBind) = 0;
        virtual void UnbindVertexArray() = 0;
        virtual void DestroyVertexArray(cVertexArray* vertexArray) = 0;
        virtual void BindShader(const cShader* shader) = 0;
        virtual void UnbindShader() = 0;
        virtual cShader* CreateShader(eCategory renderPath, const std::string& vertexPath, const std::string& fragmentPath, const std::vector<cShader::sDefinePair>& definePairs = {}) = 0;
        virtual cShader* CreateShader(const cShader* baseShader, const std::string& vertexFunc, const std::string& fragmentFunc, const std::vector<cShader::sDefinePair>& definePairs = {}) = 0;
        virtual void DefineInShader(cShader* shader, const std::vector<cShader::sDefinePair>& definePairs) = 0;
        virtual void DestroyShader(cShader* shader) = 0;
        virtual void SetShaderUniform(const cShader* shader, const std::string& name, const glm::mat4& matrix) = 0;
        virtual void SetShaderUniform(const cShader* shader, const std::string& name, types::usize count, const types::f32* values) = 0;
        virtual cRenderTarget* CreateRenderTarget(const std::vector<cTexture*>& colorAttachments, cTexture* depthAttachment) = 0;
        virtual void ResizeRenderTargetColors(cRenderTarget* renderTarget, const glm::vec2& size) = 0;
        virtual void ResizeRenderTargetDepth(cRenderTarget* renderTarget, const glm::vec2& size) = 0;
        virtual void UpdateRenderTargetBuffers(cRenderTarget* renderTarget) = 0;
        virtual void BindRenderTarget(const cRenderTarget* renderTarget) = 0;
        virtual void UnbindRenderTarget() = 0;
        virtual void DestroyRenderTarget(cRenderTarget* renderTarget) = 0;
        virtual cRenderPassGPU* CreateRenderPass(const sRenderPassDescriptor& desc) = 0;
        virtual void BindRenderPass(const cRenderPass* renderPass, cShader* customShader = nullptr) = 0;
        virtual void UnbindRenderPass(const cRenderPass* renderPass) = 0;
        virtual void DestroyRenderPass(cRenderPassGPU* renderPass) = 0;
        virtual void BindDefaultInputLayout() = 0;
        virtual void BindDepthMode(const sDepthMode& blendMode) = 0;
        virtual void BindBlendMode(const sBlendMode& blendMode) = 0;
        virtual void Viewport(const sViewport& viewport) = 0;
        virtual void ClearColor(const glm::vec4& color) = 0;
        virtual void ClearDepth(types::f32 depth) = 0;
        virtual void ClearFramebufferColor(types::usize bufferIndex, const glm::vec4& color) = 0;
        virtual void ClearFramebufferDepth(types::f32 depth) = 0;
        virtual void Draw(types::usize indexCount, types::usize vertexOffset, types::usize indexOffset, types::usize instanceCount) = 0;
        virtual void DrawQuad() = 0;
        virtual void DrawQuads(types::usize count) = 0;*/
    };
}