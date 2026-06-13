// render_pass.hpp

#pragma once

#include "object.hpp"
#include "category.hpp"
#include "rasterizer.hpp"
#include "math.hpp"

namespace triton
{
    class cBuffer;
    class cVertexArray;
    class cShader;
    class cTexture;
    class cTextureAtlasTexture;
    class cShader;
    class cRenderTarget;
    class XRenderPassGPU;

    class SRenderPassDescriptor final
    {
    public:
        enum class eRenderPath
        {
            NONE = 0,
            OPAQUE_PATH,
            TRANSPARENT_PATH,
            TEXT_PATH,
            TRANSPARENT_COMPOSITE_PATH,
            QUAD_PATH
        };

        eCategory _inputVertexFormat = eCategory::VERTEX_BUFFER_FORMAT_NONE;
        std::vector<cBuffer*> _inputBuffers = {};
        std::vector<cTexture*> _inputTextures = {};
        std::vector<std::string> _inputTextureNames = {};
        std::vector<cTextureAtlasTexture*> _inputTextureAtlasTextures = {};
        std::vector<std::string> _inputTextureAtlasTextureNames = {};
        eRenderPath _shaderRenderPath = eRenderPath::NONE;
        std::string _shaderVertexPath = "";
        std::string _shaderFragmentPath = "";
        std::string _shaderVertexFunc = "";
        std::string _shaderFragmentFunc = "";
        cShader* _shaderBase = nullptr;
        sDepthMode _depthMode = {};
        sBlendMode _blendMode = {};
        sViewport _viewport = {};
        cRenderTarget* _renderTarget = nullptr;
    };

    class XRenderPass : public iObject
    {
        TRITON_OBJECT(XRenderPass)

        SRenderPassDescriptor _desc = {};
        XRenderPassGPU* _renderPassGPU = nullptr;

    public:
        explicit XRenderPass(cContext* context, const SRenderPassDescriptor& desc, XRenderPassGPU* renderPassGPU);
        virtual ~XRenderPass() override final = default;

        void ResizeViewport(const cVector2& size);
        void ResizeColorAttachments(const cVector2& size);
        void ResizeDepthAttachment(const cVector2& size);

        inline const std::vector<cTextureAtlasTexture*>& GetInputTextureAtlasTextures() const { return _desc._inputTextureAtlasTextures; }
        cVertexArray* GetVertexArray() const;
        cShader* GetShader() const;
        inline cRenderTarget* GetRenderTarget() const { return _desc._renderTarget; }
        inline const sViewport& GetViewport() const { return _desc._viewport; }
        inline const std::vector<cBuffer*>& GetInputBuffers() const { return _desc._inputBuffers; }
        inline const std::vector<cTexture*>& GetInputTextures() const { return _desc._inputTextures; }
        inline const std::vector<std::string>& GetInputTextureNames() const { return _desc._inputTextureNames; }
        inline const sBlendMode& GetBlendMode() const { return _desc._blendMode; }
        inline const sDepthMode& GetDepthMode() const { return _desc._depthMode; }
        inline XRenderPassGPU* GetRenderPassGPU() const { return _renderPassGPU; }
        cBuffer* GetInstanceBuffer() const;
        cBuffer* GetMaterialBuffer() const;
        cBuffer* GetTextureBuffer() const;
        inline void SetInputTexture(types::usize textureIndex, cTexture* texture) { _desc._inputTextures[textureIndex] = texture; }
        inline void SetRenderTarget(cRenderTarget* newRenderTarget) { _desc._renderTarget = newRenderTarget; }
    };
}