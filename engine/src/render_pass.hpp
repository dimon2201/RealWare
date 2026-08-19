// render_pass.hpp

#pragma once

#include "object.hpp"
#include "handle.hpp"
#include "rasterizer_state.hpp"
#include "batch_data.hpp"
#include "render_target.hpp"
#include "camera.hpp"
#include "input_layout.hpp"
#include "shader.hpp"
#include "math.hpp"

namespace triton
{
    enum class ERenderPassDispatch
    {
        None,
        Geometry,
        Text,
        Processing
    };

    enum class EBuiltinRenderPassType
    {
        None,
        OpaqueStatic,
        OpaqueSkinned,
        TransparentStatic,
        Text,
        CompositeTransparent,
        CompositeFinal
    };

    struct SRenderPassTexture
    {
        SRenderPassTexture(const std::string& name, const CGPUTextureResource& texture) : name(name), texture(texture) {}

        std::string name = "";
        CGPUTextureResource texture;
    };

	struct XRenderPass : public iObject
	{
        TRITON_OBJECT(XRenderPass)

        ERenderPassDispatch                 _dispatch = ERenderPassDispatch::None;
        EVertexBufferFormat                 _batchFormat = EVertexBufferFormat::Unknown;
        std::vector<SBatchData::THandle>    _batches = {};
        XInputLayout::THandle               _inputLayout;
        std::vector<CGPUBufferResource>             _inputBuffers = {};
        std::vector<SRenderPassTexture>     _inputTextures = {};
        EBuiltinRenderPassType              _shaderRenderPath = EBuiltinRenderPassType::None;
        SDepthState                         _depthState = SDepthState(0, 0);
        SBlendState                         _blendState = {};
        SViewport                           _viewport = {};
        XRenderTarget::THandle              _renderTarget;
        XShader::THandle                    _shader;
        XCamera::THandle                    _camera;

    public:
        explicit XRenderPass(cContext* context);
        ~XRenderPass() = default;

        void Bind();

        void Unbind();

        void Render();

        void Execute();

        void ResizeViewport(const cVector2& size);

        void ResizeColorAttachments(const cVector2& size);

        void ResizeDepthAttachment(const cVector2& size);

        XShader::THandle GetShader() { return _shader; }

        void SetDispatch(ERenderPassDispatch dispatch) { _dispatch = dispatch; }

        void SetBatchFormat(EVertexBufferFormat batchFormat) { _batchFormat = batchFormat; }

        void SetBatches(const std::vector<SBatchData::THandle>& batches) { _batches = batches; }

        void SetInputLayout(const XInputLayout::THandle& inputLayout) { _inputLayout = inputLayout; }

        void SetInputBuffers(const std::vector<CGPUBufferResource>& inputBuffers) { _inputBuffers = inputBuffers; }

        void SetInputTextures(const std::vector<SRenderPassTexture>& inputTextures) { _inputTextures = inputTextures; }

        void SetDepthState(const SDepthState& depthState) { _depthState = depthState; }

        void SetBlendState(const SBlendState& blendState) { _blendState = blendState; }

        void SetViewport(const SViewport& viewport) { _viewport = viewport; }

        void SetRenderTarget(const XRenderTarget::THandle& renderTarget) { _renderTarget = renderTarget; }

        void SetShader(const XShader::THandle& shader) { _shader = shader; }

        void SetCamera(const XCamera::THandle& camera) { _camera = camera; }

        struct THandle : public SHandle {};

        struct TGPULayout {};
	};
}