// render_pass.hpp

#pragma once

#include "object.hpp"
#include "category.hpp"
#include "rasterizer.hpp"
#include "render_instance.hpp"
#include "math.hpp"
#include "render_data.hpp"
#include "instance_buffer.hpp"

namespace triton
{
    namespace ecs::components
    {
        class SRenderInstanceComponent;
    }

    class cBuffer;
    class cVertexArray;
    class cShader;
    class cTexture;
    class cTextureAtlasTexture;
    class cShader;
    class cRenderTarget;
    class XRenderPassGPU;

    enum class eDefaultRenderPath
    {
        NONE,
        OPAQUE_PATH,
        TRANSPARENT_PATH,
        TEXT_PATH,
        TRANSPARENT_COMPOSITE_PATH,
        QUAD_PATH
    };

    class SRenderPassTexture final
    {
    public:
        cTextureAtlasTexture* _texture = nullptr;
        std::string _name = "";
    };

    class XRenderPass : public iObject
    {
        TRITON_OBJECT(XRenderPass)

        eCategory                           _inputVertexFormat = eCategory::VERTEX_BUFFER_FORMAT_NONE;
        std::vector<cBuffer*>               _inputBuffers = {};
        std::vector<cTexture*>              _inputTextures = {};
        std::vector<std::string>            _inputTextureNames = {};
        eDefaultRenderPath                  _shaderRenderPath = eDefaultRenderPath::NONE;
        std::string                         _shaderVertexPath = "";
        std::string                         _shaderFragmentPath = "";
        std::string                         _shaderVertexFunc = "";
        std::string                         _shaderFragmentFunc = "";
        cShader*                            _shaderBase = nullptr;
        sDepthMode                          _depthMode = {};
        sBlendMode                          _blendMode = {};
        sViewport                           _viewport = {};
        cRenderTarget*                      _renderTarget = nullptr;
        cVertexArray*                       _vertexArray = nullptr;
        cShader*                            _shader = nullptr;
        XInstanceBuffer*                    _instanceBufferStatic = nullptr;
        XInstanceBuffer*                    _instanceBufferDynamic = nullptr;
        cBuffer*                            _materialBuffer = nullptr;
        cBuffer*                            _textureBuffer = nullptr;
        cStack<SInstanceBufferHandle>*      _dirtyStaticInstances = nullptr;

        void WriteDirtyStaticInstancesToGPU();
        void WriteDynamicInstancesToGPU();

    public:
        explicit XRenderPass(cContext* context);
        virtual ~XRenderPass() override = default;

        void WriteStaticInstanceToGPU(const SInstanceBufferHandle& instance);
        void SynchronizeGPU();
        void Execute();
        void ResizeViewport(const cVector2& size);
        void ResizeColorAttachments(const cVector2& size);
        void ResizeDepthAttachment(const cVector2& size);
        void SetInputTextures(const std::vector<SRenderPassTexture>& textures);

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
        XInstanceBuffer* GetInstanceBuffer() const;
        cBuffer* GetMaterialBuffer() const;
        cBuffer* GetTextureBuffer() const;
        inline void SetInputTexture(types::usize textureIndex, cTexture* texture) { _desc._inputTextures[textureIndex] = texture; }
        inline void SetRenderTarget(cRenderTarget* newRenderTarget) { _desc._renderTarget = newRenderTarget; }
    };
}