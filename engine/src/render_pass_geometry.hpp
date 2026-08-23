// render_pass_geometry.hpp

#pragma once

#include <optional>
#include "render_pass.hpp"
#include "math.hpp"
#include "render_target.hpp"
#include "shader_texture_binding.hpp"
#include "render_instance_pack.hpp"
#include "render_shading_model_enum.hpp"
#include "input_layout.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "types.hpp"

namespace triton
{
    class XRenderPassGeometry : public IRenderPass
    {
        TRITON_CLASS_NAME(XRenderPassGeometry)

        std::optional<SClearState>                  _clearState = std::nullopt;
        std::vector<XRenderInstancePack::THandle>   _renderInstancePacks = {};
        XInputLayout::THandle                       _inputLayout;
        std::vector<CGPUBufferResource>             _inputBuffers = {};
        std::vector<SShaderTextureBinding>          _inputTextures = {};
        SDepthState                                 _depthState;
        SBlendState                                 _blendState;
        SViewport                                   _viewport = {};
        XRenderTarget::THandle                      _renderTarget;
        XShader::THandle                            _shader;
        XCamera::THandle                            _camera;
        EShadingModel                               _shadingModel = EShadingModel::PBR;

    public:
        explicit XRenderPassGeometry(
            cContext* context,
            types::s32 poolIndex
        ) : IRenderPass(context, poolIndex, ERenderPassDispatch::Geometry) {}
        ~XRenderPassGeometry() override = default;

        void Render() override;

        XShader::THandle GetShader() { return _shader; }

        inline EShadingModel GetShadingModel() const { return _shadingModel; }

        void SetClearState(const std::optional<SClearState>& clearState) { _clearState = clearState; }

        void SetRenderInstancePacks(const std::vector<XRenderInstancePack::THandle>& packs) { _renderInstancePacks = packs; }

        void SetInputLayout(const XInputLayout::THandle& inputLayout) { _inputLayout = inputLayout; }

        void SetInputBuffers(const std::vector<CGPUBufferResource>& inputBuffers) { _inputBuffers = inputBuffers; }

        void SetInputTextures(const std::vector<SShaderTextureBinding>& inputTextures) { _inputTextures = inputTextures; }

        void SetDepthState(const SDepthState& depthState) { _depthState = depthState; }

        void SetBlendState(const SBlendState& blendState) { _blendState = blendState; }

        void SetViewport(const SViewport& viewport) { _viewport = viewport; }

        void SetRenderTarget(const XRenderTarget::THandle& renderTarget) { _renderTarget = renderTarget; }

        void SetShader(const XShader::THandle& shader) { _shader = shader; }

        void SetCamera(const XCamera::THandle& camera) { _camera = camera; }

        void SetShadingModel(EShadingModel shadingModel) { _shadingModel = shadingModel; }

        struct THandle : public SHandle {};

        struct TGPULayout {};

    private:
        void Bind();
        void Draw();
        void Unbind();
    };
}