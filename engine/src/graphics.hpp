// graphics.hpp

#pragma once

#include <optional>
#include <unordered_map>
#include "object.hpp"
#include "input_layout.hpp"
#include "render_target.hpp"
#include "render_pass_geometry.hpp"
#include "render_pass_processing.hpp"
#include "math.hpp"
#include "subsystem.hpp"
#include "types.hpp"

namespace triton
{
    class XVertexArrayPool;
    class XRenderTargetPool;
    class XRenderPassPool;

	class CGraphics : public CSubsystem
	{
        TRITON_CLASS_NAME(CGraphics)

        XInputLayout::THandle               _inputLayoutStatic;
        XInputLayout::THandle               _inputLayoutSkinned;
        XInputLayout::THandle               _inputLayoutProcessing;
        XShader::THandle                    _opaqueRigidPhongShader;
        XShader::THandle                    _opaqueSkinnedPhongShader;
        XShader::THandle                    _opaqueRigidPBRShader;
        XShader::THandle                    _opaqueSkinnedPBRShader;
        XShader::THandle                    _transparentShader;
        XShader::THandle                    _textShader;
        XShader::THandle                    _compositeTransparentShader;
        XShader::THandle                    _compositeFinalShader;
        XRenderTarget::THandle              _opaqueRenderTarget;
        XRenderTarget::THandle              _transparentRenderTarget;
        XRenderPassGeometry::THandle        _opaqueStatic;
        XRenderPassGeometry::THandle        _opaqueSkinned;
        XRenderPassGeometry::THandle        _transparent;
        //XRenderPass::THandle _text;
        XRenderPassProcessing::THandle      _compositeTransparent;
        XRenderPassProcessing::THandle      _compositeFinal;

	public:
        explicit CGraphics(cContext* context);
        ~CGraphics() override;

        void ExecutePasses();

        void ResizeRenderTargets(const cVector2& size);
        
        void LoadShaderFiles(const std::string& vertexFuncPath, const std::string& fragmentFuncPath, std::string& vertexFunc, std::string& fragmentFunc);
        
        void SetShadingModel(EShadingModel shadingModel);

        const XRenderPassGeometry::THandle& GetOpaqueStaticRenderPass() const { return _opaqueStatic; }

        const XRenderPassGeometry::THandle& GetOpaqueSkinnedRenderPass() const { return _opaqueSkinned; }

        inline const XShader::THandle& GetOpaqueRigidPBRShader() const { return _opaqueRigidPBRShader; }

        inline const XShader::THandle& GetOpaqueSkinnedPBRShader() const { return _opaqueSkinnedPBRShader; }

        inline const XShader::THandle& GetOpaqueRigidPhongShader() const { return _opaqueRigidPhongShader; }

        inline const XShader::THandle& GetOpaqueSkinnedPhongShader() const { return _opaqueSkinnedPhongShader; }

    private:
        void CreateInputLayouts();

        void CreateShaders();

        void CreateRenderTargets();

        void CreateRenderPasses();

        void DestroyInputLayouts();

        void DestroyShaders();

        void DestroyRenderTargets();

        void DestroyRenderPasses();

        void BindBuffers();

        void UnbindBuffers();

        void ExecuteBuiltinPasses();
	};
}