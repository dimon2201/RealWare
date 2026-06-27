// render_pass.hpp

#pragma once

#include <vector>
#include "object.hpp"
#include "category.hpp"
#include "rasterizer_state.hpp"
#include "render_instance.hpp"
#include "math.hpp"
#include "render_data.hpp"
#include "instance_buffer.hpp"
#include "graphics_buffer_formats.hpp"

namespace triton
{
    namespace ecs::components
    {
        class SRenderInstanceComponent;
    }

    class cBuffer;
    class cTexture;
    class cTextureAtlasTexture;
    class XRenderTarget;
    class XRenderPassGPU;
    class XShader;
    class XVertexArray;
    class SShaderDefine;

    enum class EBuiltinRenderPassType
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
        SRenderPassTexture(const std::string& name, cTexture* texture) : _name(name), _texture(texture) {}

        cTexture* _texture = nullptr;
        std::string _name = "";
    };

    class XRenderPass : public iObject
    {
        TRITON_OBJECT(XRenderPass)

        EGraphicsBufferFormat           _inputVertexFormat = EGraphicsBufferFormat::NONE;
        XVertexArray*                   _vertexArray = nullptr;
        std::vector<cBuffer*>           _inputBuffers = {};
        std::vector<SRenderPassTexture> _inputTextures = {};
        EBuiltinRenderPassType          _shaderRenderPath = EBuiltinRenderPassType::NONE;
        SDepthState                     _depthState = SDepthState(0, 0);
        SBlendState                     _blendState = {};
        SViewport                       _viewport = {};
        XRenderTarget*                  _renderTarget = nullptr;
        XShader*                        _shader = nullptr;
        XInstanceBuffer*                _instanceBufferStatic = nullptr;
        XInstanceBuffer*                _instanceBufferDynamic = nullptr;
        cBuffer*                        _materialBuffer = nullptr;
        cBuffer*                        _textureBuffer = nullptr;
        cStack<SInstanceBufferHandle>*  _dirtyStaticInstances = nullptr;

        void WriteDirtyStaticInstancesToGPU();
        void WriteDynamicInstancesToGPU();
        void SynchronizeWithGPU();

    public:
        explicit XRenderPass(cContext* context);
        ~XRenderPass() override;

        SInstanceBufferHandle AddInstance(SRenderInstance::EUsage usage, SRenderInstance& instance);
        void WriteStaticInstanceToGPU(SInstanceBufferHandle& instance);
        void Execute();
        void ResizeViewport(const cVector2& size);
        void ResizeColorAttachments(const cVector2& size);
        void ResizeDepthAttachment(const cVector2& size);
        SShaderDefine SetInputTexture(types::usize slot, const SRenderPassTexture& texture);
        std::vector<SShaderDefine> SetInputTextures(const std::vector<SRenderPassTexture>& textures);

        inline XRenderTarget* GetRenderTarget() const
        {
            return _renderTarget;
        }

        inline XShader* GetShader() const
        {
            return _shader;
        }

        inline void SetInputBuffers(const std::vector<cBuffer*>& buffers)
        {
            _inputBuffers = buffers;
        }

        inline void SetVertexArray(XVertexArray* vertexArray)
        {
            _vertexArray = vertexArray;
        }

        inline void SetInputVertexFormat(EGraphicsBufferFormat format)
        {
            _inputVertexFormat = format;
        }

        inline void SetDepthState(const SDepthState& state)
        {
            _depthState = state;
        }

        inline void SetBlendState(const SBlendState& state)
        {
            _blendState = state;
        }

        inline void SetViewport(const SViewport& viewport)
        {
            _viewport = viewport;
        }

        inline void SetRenderTarget(XRenderTarget* renderTarget)
        {
            _renderTarget = renderTarget;
        }

        inline void SetShader(XShader* shader)
        {
            _shader = shader;
        }
    };
}