// render_pass_processing.hpp

#pragma once

#include <vector>
#include "render_pass.hpp"
#include "shader_texture_binding.hpp"
#include "shader.hpp"
#include "rasterizer_state.hpp"
#include "render_target.hpp"
#include "gpu_buffer_resource.hpp"
#include "input_layout.hpp"
#include "types.hpp"

namespace triton
{
    class XRenderPassProcessing : public IRenderPass
    {
        TRITON_CLASS_NAME(XRenderPassProcessing)

        XInputLayout::THandle                       _inputLayout;
        std::vector<CGPUBufferResource>             _inputBuffers = {};
        std::vector<SShaderTextureBinding>          _inputTextures = {};
        SDepthState                                 _depthState = SDepthState(0, 0);
        SBlendState                                 _blendState = {};
        SViewport                                   _viewport = {};
        XRenderTarget::THandle                      _renderTarget;
        XShader::THandle                            _shader;

    public:
        explicit XRenderPassProcessing(
            cContext* context,
            types::s32 poolIndex,
            const XInputLayout::THandle& inputLayout,
            const std::vector<CGPUBufferResource>& inputBuffers,
            const std::vector<SShaderTextureBinding>& inputTextures,
            const SViewport& viewport,
            const SBlendState& blendState,
            const SDepthState& depthState,
            const XRenderTarget::THandle& renderTarget,
            const XShader::THandle& shader
        );
        ~XRenderPassProcessing() override = default;

        void Render() override;

        inline const std::vector<CGPUBufferResource>& GetInputBuffers() const { return _inputBuffers; }

        inline const std::vector<SShaderTextureBinding>& GetInputTextures() const { return _inputTextures; }

        inline const XShader::THandle& GetShader() const { return _shader; }

        struct THandle : public SHandle {};

        struct TGPULayout {};
    };
}