// graphics.hpp

#pragma once

#include <optional>
#include <unordered_map>
#include "object.hpp"
#include "input_layout.hpp"
#include "render_target.hpp"
#include "render_pass.hpp"
#include "math.hpp"
#include "types.hpp"

namespace triton
{
    class XVertexArrayPool;
    class XRenderTargetPool;
    class XRenderPassPool;

	class XGraphics : public iObject
	{
        TRITON_OBJECT(XGraphics)

        XInputLayout::THandle _inputLayoutStatic;
        XInputLayout::THandle _inputLayoutSkinned;
        XInputLayout::THandle _inputLayoutProcessing;
        XRenderTarget::THandle _opaqueRenderTarget;
        XRenderTarget::THandle _transparentRenderTarget;
        XRenderPass::THandle _opaqueStatic;
        XRenderPass::THandle _opaqueSkinned;
        XRenderPass::THandle _transparent;
        XRenderPass::THandle _text;
        XRenderPass::THandle _compositeTransparent;
        XRenderPass::THandle _compositeFinal;

	public:
        explicit XGraphics(cContext* context);
        ~XGraphics() override;

        void ExecutePasses();

        void ResizeRenderTargets(const cVector2& size);
        
        void LoadShaderFiles(const std::string& vertexFuncPath, const std::string& fragmentFuncPath, std::string& vertexFunc, std::string& fragmentFunc);
        
        const XRenderPass::THandle& GetOpaqueStaticRenderPass() const { return _opaqueStatic; }

        const XRenderPass::THandle& GetOpaqueSkinnedRenderPass() const { return _opaqueSkinned; }

    private:
        void CreateInputLayouts();

        void CreateRenderTargets();

        void CreateRenderPasses();

        void DestroyInputLayouts();

        void DestroyRenderTargets();

        void DestroyRenderPasses();

        void BindBuffers();

        void UnbindBuffers();

        void ExecuteBuiltinPasses();
	};
}