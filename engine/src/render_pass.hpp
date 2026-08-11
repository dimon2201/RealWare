// render_pass.hpp

#pragma once

#include <vector>
#include "object.hpp"
#include "category.hpp"
#include "rasterizer_state.hpp"
#include "render_instance_data.hpp"
#include "math.hpp"
#include "render_data.hpp"
#include "instance_buffer.hpp"
#include "graphics_buffer_formats.hpp"
#include "camera.hpp"
#include "vertex_buffer_format.hpp"

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
    class XRenderBatch;

    enum class EBuiltinRenderPassType
    {
        NONE,
        OPAQUE_PATH,
        TRANSPARENT_PATH,
        TEXT_PATH,
        TRANSPARENT_COMPOSITE_PATH,
        QUAD_PATH
    };

    enum class ERenderPassDispatch
    {
        NONE,
        GEOMETRY,
        TEXT,
        COMPUTE,
        PROCESSING
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

        ERenderPassDispatch                 _dispatch = ERenderPassDispatch::NONE;
        EVertexBufferFormat                 _batchFormat = EVertexBufferFormat::Unknown;
        std::vector<SBatchData::THandle>    _batches = {};
        XVertexArray*                       _inputLayout = nullptr;
        std::vector<cBuffer*>               _inputBuffers = {};
        std::vector<SRenderPassTexture>     _inputTextures = {};
        EBuiltinRenderPassType              _shaderRenderPath = EBuiltinRenderPassType::NONE;
        SDepthState                         _depthState = SDepthState(0, 0);
        SBlendState                         _blendState = {};
        SViewport                           _viewport = {};
        XRenderTarget*                      _renderTarget = nullptr;
        XShader*                            _shader = nullptr;
        XCamera::THandle                    _camera;

    public:
        explicit XRenderPass(cContext* context) : iObject(context) {}
        ~XRenderPass() override = default;

        void Bind();

        void Unbind();

        void Render();

        void Execute();

        void ResizeViewport(const cVector2& size);

        void ResizeColorAttachments(const cVector2& size);

        void ResizeDepthAttachment(const cVector2& size);

        SShaderDefine SetInputTexture(types::usize slot, const SRenderPassTexture& texture);

        std::vector<SShaderDefine> SetInputTextures(const std::vector<SRenderPassTexture>& textures);

        inline void AddBatch(const SBatchData::THandle& batch)
        {
            SBatchData& bd = _context->GetSubsystem<XBatchSubsystem>()->Get(batch);
            if (bd.vertexFormat == _batchFormat)
                _batches.push_back(batch);
        }

        inline XRenderTarget* GetRenderTarget() const
        {
            return _renderTarget;
        }

        inline XShader* GetShader() const
        {
            return _shader;
        }

        inline void SetBatchFormat(EVertexBufferFormat format)
        {
            _batchFormat = format;
        }

        inline void SetDispatch(ERenderPassDispatch dispatch)
        {
            _dispatch = dispatch;
        }

        inline void SetInputLayout(XVertexArray* inputLayout)
        {
            _inputLayout = inputLayout;
        }

        inline void SetInputBuffers(const std::vector<cBuffer*>& buffers)
        {
            _inputBuffers = buffers;
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

        inline void SetCamera(const XCamera::THandle& camera)
        {
            _camera = camera;
        }
    };
}