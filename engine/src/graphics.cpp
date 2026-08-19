// graphics.cpp

#include "graphics.hpp"
#include "thread_guard.hpp"
#include "graphics_backend.hpp"
#include "render_pass_pool.hpp"
#include "render_target_pool.hpp"
#include "input_layout_pool.hpp"
#include "filesystem_manager.hpp"
#include "input.hpp"
#include "geometry_storage.hpp"
#include "material_subsystem.hpp"
#include "batcher.hpp"
#include "skinning_subsystem.hpp"
#include "skinned_bones_pool.hpp"
#include "texture_atlas.hpp"
#include "shader_pool.hpp"

using namespace types;

triton::XGraphics::XGraphics(cContext* context) : iObject(context)
{
    CreateInputLayouts();
    CreateRenderTargets();
    CreateRenderPasses();
}

triton::XGraphics::~XGraphics()
{
    DestroyRenderPasses();
    DestroyRenderTargets();
    DestroyInputLayouts();
}

void triton::XGraphics::ExecutePasses()
{
    CThreadGuard::AssertRender();

    BindBuffers();
    ExecuteBuiltinPasses();
    UnbindBuffers();
}

void triton::XGraphics::ResizeRenderTargets(const cVector2& size)
{
    // TODO: finish this to make resizing work
    // |||||||||||||||||||||||||||||||||||||||
    // VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV

    //UnbindPass(_opaqueStatic);
    //UnbindPass(_opaqueSkinned);
    //UnbindPass(_transparent);

    //ResizePassViewport(_opaqueStatic);
    //ResizePassViewport(_opaqueSkinned);
    //ResizePassAttachments(_opaqueStatic);
    //ResizePassViewport(_transparent);
    //ResizePassAttachments(_transparent);
    //ResizePassViewport(_text);
    //ResizePassViewport(_compositeTransparent);
    //ResizePassViewport(_compositeFinal);

    //SRenderPass& opaqueStaticData = *_renderPassPool->Get(_opaqueStatic);
    //SRenderTarget& opaqueStaticRTData = *_renderTargetPool->Get(opaqueStaticData.renderTarget);
    //SRenderPass& transparentData = *_renderPassPool->Get(_transparent);
    //SRenderTarget& transparentRTData = *_renderTargetPool->Get(transparentData.renderTarget);

    //transparentRTData._renderTarget->SetDepthAttachment(opaqueStaticRTData._renderTarget->GetDepthAttachment());

    //XRenderTarget* newOpaqueRenderTarget = _opaqueStatic->GetRenderTarget();
    //XRenderTarget* newTransparentRenderTarget = _transparent->GetRenderTarget();
    //gfxPipelineBackend->UpdateRenderTargetBuffers(newOpaqueRenderTarget);
    //gfxPipelineBackend->UpdateRenderTargetBuffers(newTransparentRenderTarget);
    //_opaqueStatic->SetRenderTarget(newOpaqueRenderTarget);
    //_opaqueSkinned->SetRenderTarget(newOpaqueRenderTarget);
    //_transparent->SetRenderTarget(newTransparentRenderTarget);

    //auto& transparentColorAttachments = _transparent->GetRenderTarget()->GetColorAttachments();
    //_compositeTransparent->SetInputTexture(0, SRenderPassTexture("AccumulationTexture", transparentColorAttachments[0]));
    //_compositeTransparent->SetInputTexture(1, SRenderPassTexture("RevealageTexture", transparentColorAttachments[1]));

    //auto& opaqueColorAttachments = _opaqueStatic->GetRenderTarget()->GetColorAttachments();
    //_compositeFinal->SetInputTexture(0, SRenderPassTexture("ColorTexture", opaqueColorAttachments[0]));
}

void triton::XGraphics::LoadShaderFiles(
    const std::string& vertexFuncPath,
    const std::string& fragmentFuncPath,
    std::string& vertexFunc,
    std::string& fragmentFunc
)
{
    cFileSystem* fileSystem = _context->GetSubsystem<cFileSystem>();

    cDataFile* vertexFuncFile = fileSystem->CreateDataFile(vertexFuncPath, K_TRUE);
    cDataFile* fragmentFuncFile = fileSystem->CreateDataFile(fragmentFuncPath, K_TRUE);

    vertexFunc = std::string((const char*)vertexFuncFile->GetData());
    fragmentFunc = std::string((const char*)fragmentFuncFile->GetData());

    fileSystem->DestroyDataFile(vertexFuncFile);
    fileSystem->DestroyDataFile(fragmentFuncFile);
}

void triton::XGraphics::CreateInputLayouts()
{
    XGeometryStorage* gs = _context->GetSubsystem<XGeometryStorage>();
    const std::vector<CGPUBuffer> staticInputs = {
        gs->GetStaticVertexBuffer(),
        gs->GetStaticIndexBuffer()
    };
    const std::vector<CGPUBuffer> skinnedInputs = {
        gs->GetSkinnedVertexBuffer(),
        gs->GetSkinnedIndexBuffer()
    };
    XInputLayoutPool* iaPool = _context->GetPool<XInputLayoutPool>();
    _inputLayoutStatic = *iaPool->Create(_context, staticInputs, EVertexBufferFormat::Static_52);
    _inputLayoutSkinned = *iaPool->Create(_context, skinnedInputs, EVertexBufferFormat::Skinned_84);
    _inputLayoutProcessing = *iaPool->Create(_context, std::vector<CGPUBuffer>(), EVertexBufferFormat::Unknown);
}

void triton::XGraphics::CreateRenderTargets()
{
    cVector2 windowSize = _context->GetSubsystem<cInput>()->GetWindows()->at(0).GetSize();
    _context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::CREATE_TEXTURE,
        windowSize.GetX(),
        windowSize.GetY(),
        0,
        (cpuword)ETextureDimension::Texture2D,
        (cpuword)ETextureFormat::RGBA8,
        (cpuword)nullptr,
        0
    ));
    CGPUTextureResource color = _context->GetSubsystem<cEngine>()->GetSynchronization()->WaitForRenderCommandResult<CGPUTextureResource>();
    _context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::CREATE_TEXTURE,
        windowSize.GetX(),
        windowSize.GetY(),
        0,
        (cpuword)ETextureDimension::Texture2D,
        (cpuword)ETextureFormat::RGBA16F,
        (cpuword)nullptr,
        0
    ));
    CGPUTextureResource accumulation = _context->GetSubsystem<cEngine>()->GetSynchronization()->WaitForRenderCommandResult<CGPUTextureResource>();
    _context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::CREATE_TEXTURE,
        windowSize.GetX(),
        windowSize.GetY(),
        0,
        (cpuword)ETextureDimension::Texture2D,
        (cpuword)ETextureFormat::R8F,
        (cpuword)nullptr,
        0
    ));
    CGPUTextureResource revealage = _context->GetSubsystem<cEngine>()->GetSynchronization()->WaitForRenderCommandResult<CGPUTextureResource>();
    _context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::CREATE_TEXTURE,
        windowSize.GetX(),
        windowSize.GetY(),
        0,
        (cpuword)ETextureDimension::Texture2D,
        (cpuword)ETextureFormat::DepthStencil,
        (cpuword)nullptr,
        0
    ));
    CGPUTextureResource depth = _context->GetSubsystem<cEngine>()->GetSynchronization()->WaitForRenderCommandResult<CGPUTextureResource>();
    
    XRenderTargetPool* rtPool = _context->GetPool<XRenderTargetPool>();

    CGPUTextureResource opaqueColorAttachments[1] = { color };
    _context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::CREATE_RENDER_TARGET,
        1,
        (cpuword)&opaqueColorAttachments[0],
        (cpuword)&depth
    ));
    CGPURenderTarget opaqueRT = _context->GetSubsystem<cEngine>()->GetSynchronization()->WaitForRenderCommandResult<CGPURenderTarget>();
    _opaqueRenderTarget = *rtPool->Create(_context, opaqueRT);

    CGPUTextureResource transparentColorAttachments[2] = { accumulation, revealage };
    _context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::CREATE_RENDER_TARGET,
        2,
        (cpuword)&transparentColorAttachments[0],
        (cpuword)&depth
    ));
    CGPURenderTarget transparentRT = _context->GetSubsystem<cEngine>()->GetSynchronization()->WaitForRenderCommandResult<CGPURenderTarget>();
    _transparentRenderTarget = *rtPool->Create(_context, transparentRT);
}

void triton::XGraphics::CreateRenderPasses()
{
    cVector2 windowSize = _context->GetSubsystem<cInput>()->GetWindows()->at(0).GetSize();
    XTextureAtlas* textureAtlas = _context->GetSubsystem<XTextureAtlas>();
    cFileSystem* fs = _context->GetSubsystem<cFileSystem>();

    SViewport viewport;
    viewport.rect = cVector4(0.0f, 0.0f, windowSize.GetX(), windowSize.GetY());

    const std::string pbrShaderPath = "C:/My/My_Projects_Programming/TritonEngine/runtime/data/shaders/builtin/brdf.shader";
    const std::string opaqueStaticVertexShaderPath = "C:/My/My_Projects_Programming/TritonEngine/runtime/data/shaders/builtin/opaque_vertex_static.shader";
    const std::string opaqueSkinnedVertexShaderPath = "C:/My/My_Projects_Programming/TritonEngine/runtime/data/shaders/builtin/opaque_vertex_skinned.shader";
    const std::string opaqueFragmentShaderPath = "C:/My/My_Projects_Programming/TritonEngine/runtime/data/shaders/builtin/opaque_fragment.shader";
    const std::string transparentVertexShaderPath = "C:/My/My_Projects_Programming/TritonEngine/runtime/data/shaders/builtin/transparent_vertex.shader";
    const std::string transparentFragmentShaderPath = "C:/My/My_Projects_Programming/TritonEngine/runtime/data/shaders/builtin/transparent_fragment.shader";
    const std::string textVertexShaderPath = "C:/My/My_Projects_Programming/TritonEngine/runtime/data/shaders/builtin/text_vertex.shader";
    const std::string textFragmentShaderPath = "C:/My/My_Projects_Programming/TritonEngine/runtime/data/shaders/builtin/text_fragment.shader";
    const std::string compositeTransparentVertexShaderPath = "C:/My/My_Projects_Programming/TritonEngine/runtime/data/shaders/builtin/composite_transparent_vertex.shader";
    const std::string compositeTransparentFragmentShaderPath = "C:/My/My_Projects_Programming/TritonEngine/runtime/data/shaders/builtin/composite_transparent_fragment.shader";
    const std::string compositeFinalVertexShaderPath = "C:/My/My_Projects_Programming/TritonEngine/runtime/data/shaders/builtin/composite_final_vertex.shader";
    const std::string compositeFinalFragmentShaderPath = "C:/My/My_Projects_Programming/TritonEngine/runtime/data/shaders/builtin/composite_final_fragment.shader";

    XRenderPassPool* rpPool = _context->GetPool<XRenderPassPool>();
    std::string emptyStr = "";
    std::vector<SShaderDefine> emptyShaderDefineVec = {};
    std::vector<const char*> emptyConstCharVec = {};

    // Opaque static render pass
    const std::vector<const char*> opaqueShaderFragmentIncludePaths = { pbrShaderPath.c_str()};
    auto opaqueStaticVertexStr = fs->TextFileToString(opaqueStaticVertexShaderPath);
    auto opaqueStaticFragmentStr = fs->TextFileToString(opaqueFragmentShaderPath);
    XShader::THandle opaqueStaticShader = *_context->GetPool<XShaderPool>()->Create(
        _context,
        opaqueStaticVertexStr,
        opaqueStaticFragmentStr,
        emptyStr,
        emptyStr,
        emptyShaderDefineVec,
        emptyConstCharVec,
        opaqueShaderFragmentIncludePaths
    );
    
    SBlendState opaqueBlendState = {};
    opaqueBlendState.factorCount = 1;
    opaqueBlendState.srcFactors[0] = EBlendFactor::ONE;
    opaqueBlendState.dstFactors[0] = EBlendFactor::ZERO;
    
    _opaqueStatic = *rpPool->Create(_context);
    XRenderPass& opaqueStaticData = *rpPool->Get(_opaqueStatic);
    opaqueStaticData.SetDispatch(ERenderPassDispatch::Geometry);
    opaqueStaticData.SetBatchFormat(EVertexBufferFormat::Static_52);
    opaqueStaticData.SetInputLayout(_inputLayoutStatic);
    opaqueStaticData.SetInputTextures({
        SRenderPassTexture("TextureAtlasRGBA8SRGB", textureAtlas->GetAtlasRGBA8SRGB()),
        SRenderPassTexture("TextureAtlasRGBA8", textureAtlas->GetAtlasRGBA8()),
        SRenderPassTexture("TextureAtlasR8", textureAtlas->GetAtlasR8())
    });
    opaqueStaticData.SetShader(opaqueStaticShader);
    opaqueStaticData.SetViewport(viewport);
    opaqueStaticData.SetDepthState(SDepthState(K_TRUE, K_TRUE));
    opaqueStaticData.SetBlendState(opaqueBlendState);
    opaqueStaticData.SetRenderTarget(_opaqueRenderTarget);

    // Opaque skinned render pass
    auto opaqueSkinnedVertexStr = fs->TextFileToString(opaqueSkinnedVertexShaderPath);
    auto opaqueSkinnedFragmentStr = fs->TextFileToString(opaqueFragmentShaderPath);
    XShader::THandle opaqueSkinnedShader = *_context->GetPool<XShaderPool>()->Create(
        _context,
        opaqueSkinnedVertexStr,
        opaqueSkinnedFragmentStr,
        emptyStr,
        emptyStr,
        emptyShaderDefineVec,
        emptyConstCharVec,
        opaqueShaderFragmentIncludePaths
    );

    _opaqueSkinned = *rpPool->Create(_context);
    XRenderPass& opaqueSkinnedData = *rpPool->Get(_opaqueSkinned);
    opaqueSkinnedData.SetDispatch(ERenderPassDispatch::Geometry);
    opaqueSkinnedData.SetBatchFormat(EVertexBufferFormat::Skinned_84);
    opaqueSkinnedData.SetInputLayout(_inputLayoutSkinned);
    opaqueSkinnedData.SetInputTextures({
        SRenderPassTexture("TextureAtlasRGBA8SRGB", textureAtlas->GetAtlasRGBA8SRGB()),
        SRenderPassTexture("TextureAtlasRGBA8", textureAtlas->GetAtlasRGBA8()),
        SRenderPassTexture("TextureAtlasR8", textureAtlas->GetAtlasR8())
    });
    opaqueSkinnedData.SetShader(opaqueSkinnedShader);
    opaqueSkinnedData.SetViewport(viewport);
    opaqueSkinnedData.SetDepthState(SDepthState(K_TRUE, K_TRUE));
    opaqueSkinnedData.SetBlendState(opaqueBlendState);
    opaqueSkinnedData.SetRenderTarget(_opaqueRenderTarget);

    // Transparent render pass
    auto transparentVertexStr = fs->TextFileToString(transparentVertexShaderPath);
    auto transparentFragmentStr = fs->TextFileToString(transparentFragmentShaderPath);
    XShader::THandle transparentShader = *_context->GetPool<XShaderPool>()->Create(
        _context,
        transparentVertexStr,
        transparentFragmentStr,
        emptyStr,
        emptyStr,
        emptyShaderDefineVec,
        emptyConstCharVec,
        emptyConstCharVec
    );

    SBlendState transparentBlendState = {};
    transparentBlendState.factorCount = 2;
    transparentBlendState.srcFactors[0] = EBlendFactor::ONE;
    transparentBlendState.dstFactors[0] = EBlendFactor::ONE;
    transparentBlendState.srcFactors[1] = EBlendFactor::ZERO;
    transparentBlendState.dstFactors[1] = EBlendFactor::INV_SRC_COLOR;

    _transparent = *rpPool->Create(_context);
    XRenderPass& transparentData = *rpPool->Get(_transparent);
    transparentData.SetDispatch(ERenderPassDispatch::Geometry);
    transparentData.SetBatchFormat(EVertexBufferFormat::Static_52);
    transparentData.SetInputLayout(_inputLayoutStatic);
    transparentData.SetInputTextures({
        SRenderPassTexture("TextureAtlasRGBA8SRGB", textureAtlas->GetAtlasRGBA8SRGB()),
        SRenderPassTexture("TextureAtlasRGBA8", textureAtlas->GetAtlasRGBA8()),
        SRenderPassTexture("TextureAtlasR8", textureAtlas->GetAtlasR8())
    });
    transparentData.SetShader(transparentShader);
    transparentData.SetViewport(viewport);
    transparentData.SetDepthState(SDepthState(K_TRUE, K_FALSE));
    transparentData.SetBlendState(transparentBlendState);
    transparentData.SetRenderTarget(_transparentRenderTarget);

    // Text render pass
    auto textVertexStr = fs->TextFileToString(textVertexShaderPath);
    auto textFragmentStr = fs->TextFileToString(textFragmentShaderPath);
    XShader::THandle textShader = *_context->GetPool<XShaderPool>()->Create(
        _context,
        textVertexStr,
        textFragmentStr,
        emptyStr,
        emptyStr,
        emptyShaderDefineVec,
        emptyConstCharVec,
        emptyConstCharVec
    );

    _text = *rpPool->Create(_context);
    XRenderPass& textData = *rpPool->Get(_text);
    textData.SetDispatch(ERenderPassDispatch::Text);
    textData.SetBatchFormat(EVertexBufferFormat::Unknown);
    textData.SetShader(textShader);
    textData.SetViewport(viewport);
    textData.SetDepthState(SDepthState(K_FALSE, K_FALSE));
    textData.SetRenderTarget(_opaqueRenderTarget);

    // Composite transparent render pass
    auto compositeTransparentVertexStr = fs->TextFileToString(compositeTransparentVertexShaderPath);
    auto compositeTransparentFragmentStr = fs->TextFileToString(compositeTransparentFragmentShaderPath);
    XShader::THandle compositeTransparentShader = *_context->GetPool<XShaderPool>()->Create(
        _context,
        compositeTransparentVertexStr,
        compositeTransparentFragmentStr,
        emptyStr,
        emptyStr,
        emptyShaderDefineVec,
        emptyConstCharVec,
        emptyConstCharVec
    );

    SBlendState compositeTransparentBlendState = {};
    compositeTransparentBlendState.factorCount = 1;
    compositeTransparentBlendState.srcFactors[0] = EBlendFactor::SRC_ALPHA;
    compositeTransparentBlendState.dstFactors[0] = EBlendFactor::INV_SRC_ALPHA;

    XRenderTarget& transparentRTData = *_context->GetPool<XRenderTargetPool>()->Get(_transparentRenderTarget);

    _compositeTransparent = *rpPool->Create(_context);
    XRenderPass& compositeTransparentData = *rpPool->Get(_compositeTransparent);
    compositeTransparentData.SetDispatch(ERenderPassDispatch::Processing);
    compositeTransparentData.SetBatchFormat(EVertexBufferFormat::Unknown);
    compositeTransparentData.SetInputLayout(_inputLayoutProcessing);
    compositeTransparentData.SetInputTextures({
        SRenderPassTexture("AccumulationTexture", transparentRTData._renderTarget.GetColorAttachments()[0]),
        SRenderPassTexture("RevealageTexture", transparentRTData._renderTarget.GetColorAttachments()[1])
    });
    compositeTransparentData.SetShader(compositeTransparentShader);
    compositeTransparentData.SetViewport(viewport);
    compositeTransparentData.SetDepthState(SDepthState(K_FALSE, K_FALSE));
    compositeTransparentData.SetBlendState(compositeTransparentBlendState);
    compositeTransparentData.SetRenderTarget(_opaqueRenderTarget);

    // Composite final render pass
    auto compositeFinalVertexStr = fs->TextFileToString(compositeFinalVertexShaderPath);
    auto compositeFinalFragmentStr = fs->TextFileToString(compositeFinalFragmentShaderPath);
    XShader::THandle compositeFinalShader = *_context->GetPool<XShaderPool>()->Create(
        _context,
        compositeFinalVertexStr,
        compositeFinalFragmentStr,
        emptyStr,
        emptyStr,
        emptyShaderDefineVec,
        emptyConstCharVec,
        emptyConstCharVec
    );

    SBlendState compositeFinalBlendState = {};
    compositeFinalBlendState.factorCount = 1;
    compositeFinalBlendState.srcFactors[0] = EBlendFactor::ONE;
    compositeFinalBlendState.dstFactors[0] = EBlendFactor::ZERO;

    XRenderTarget& opaqueRTData = *_context->GetPool<XRenderTargetPool>()->Get(_opaqueRenderTarget);

    _compositeFinal = *rpPool->Create(_context);
    XRenderPass& compositeFinalData = *rpPool->Get(_compositeFinal);
    compositeFinalData.SetDispatch(ERenderPassDispatch::Processing);
    compositeFinalData.SetBatchFormat(EVertexBufferFormat::Unknown);
    compositeFinalData.SetInputLayout(_inputLayoutProcessing);
    compositeFinalData.SetInputTextures({
        SRenderPassTexture("ColorTexture", opaqueRTData._renderTarget.GetColorAttachments()[0])
    });
    compositeFinalData.SetShader(compositeFinalShader);
    compositeFinalData.SetViewport(viewport);
    compositeFinalData.SetDepthState(SDepthState(K_FALSE, K_FALSE));
    compositeFinalData.SetBlendState(compositeFinalBlendState);
}

void triton::XGraphics::DestroyInputLayouts()
{
    XInputLayoutPool* iaPool = _context->GetPool<XInputLayoutPool>();
    iaPool->Destroy(_inputLayoutProcessing);
    iaPool->Destroy(_inputLayoutSkinned);
    iaPool->Destroy(_inputLayoutStatic);
}

void triton::XGraphics::DestroyRenderTargets()
{
    XRenderTargetPool* rtPool = _context->GetPool<XRenderTargetPool>();

    XRenderTarget& opaqueRT = *rtPool->Get(_opaqueRenderTarget);
    XRenderTarget& transparentRT = *rtPool->Get(_transparentRenderTarget);

    _context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::DESTROY_TEXTURE,
        (cpuword)&opaqueRT._renderTarget.GetColorAttachments()[0]
    ));
    _context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::DESTROY_TEXTURE,
        (cpuword)&transparentRT._renderTarget.GetColorAttachments()[0]
    ));
    _context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::DESTROY_TEXTURE,
        (cpuword)&transparentRT._renderTarget.GetColorAttachments()[1]
    ));
    _context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::DESTROY_TEXTURE,
        (cpuword)&opaqueRT._renderTarget.GetDepthAttachment()
    ));
    _context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::DESTROY_RENDER_TARGET,
        (cpuword)&transparentRT._renderTarget
    ));
    _context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::DESTROY_RENDER_TARGET,
        (cpuword)&opaqueRT._renderTarget
    ));
}

void triton::XGraphics::DestroyRenderPasses()
{
    XRenderPassPool* rpPool = _context->GetPool<XRenderPassPool>();

    rpPool->Destroy(_compositeFinal);
    rpPool->Destroy(_compositeTransparent);
    rpPool->Destroy(_text);
    rpPool->Destroy(_transparent);
    rpPool->Destroy(_opaqueSkinned);
    rpPool->Destroy(_opaqueStatic);
}

void triton::XGraphics::BindBuffers()
{
    //_context->GetSubsystem<XMaterialSubsystem>()->GetPool()->GetGPUBuffer()->Bind();
    //_context->GetSubsystem<XBatchSubsystem>()->GetStaticRenderInstancePool()->GetGPUBuffer()->Bind();
    //_context->GetSubsystem<XBatchSubsystem>()->GetDynamicRenderInstancePool()->GetGPUBuffer()->Bind();
    //_context->GetSubsystem<XSkinningSubsystem>()->GetSkinnedBonesPool()->GetGPUBuffer()->Bind();
}

void triton::XGraphics::UnbindBuffers()
{
    //_context->GetSubsystem<XSkinningSubsystem>()->GetSkinnedBonesPool()->GetGPUBuffer()->Unbind();
    //_context->GetSubsystem<XBatchSubsystem>()->GetDynamicRenderInstancePool()->GetGPUBuffer()->Unbind();
    //_context->GetSubsystem<XBatchSubsystem>()->GetStaticRenderInstancePool()->GetGPUBuffer()->Unbind();
    //_context->GetSubsystem<XMaterialSubsystem>()->GetPool()->GetGPUBuffer()->Unbind();
}

void triton::XGraphics::ExecuteBuiltinPasses()
{
    XRenderPassPool* rpPool = _context->GetPool<XRenderPassPool>();

    XRenderPass& opaqueStatic = *rpPool->Get(_opaqueStatic);
    XRenderPass& compositeFinal = *rpPool->Get(_compositeFinal);

    opaqueStatic.Execute();
    //_opaqueSkinned->Execute();
    compositeFinal.Execute();
}

// TODO: Implement new text drawing approach
/*void cGraphics::DrawTexts(const std::vector<cGameObject>& objects)
{
    for (auto& it : objects)
    {
        if (it.GetText() == nullptr)
            continue;

        const cText* text = it.GetText();
        const cFontFace* textFont = text->GetFont();
        const std::string& textString = text->GetText();
        const auto& alphabet = textFont->GetAlphabet();
        const cTexture* atlas = text->GetFont()->GetAtlas();

        _textInstancesByteSize = 0;
        _materialsMap->clear();

        const cTransform transform(&it);

        IApplication* app = _context->GetSubsystem<cEngine>()->GetApplication();
        const glm::vec2 windowSize = app->GetWindow()->GetSize();
        const glm::vec2 textPosition = glm::vec2((transform._position.x * 2.0f) - 1.0f, (transform._position.y * 2.0f) - 1.0f);
        const glm::vec2 textScale = glm::vec2(
            (1.0f / windowSize.x) * it.GetTransform()->_scale.x,
            (1.0f / windowSize.y) * it.GetTransform()->_scale.y
        );

        const usize charCount = textString.length();
        usize actualCharCount = 0;
        glm::vec2 offset = glm::vec2(0.0f);
        for (usize i = 0; i < charCount; i++)
        {
            const u8 glyphChar = textString[i];

            if (glyphChar == '\t')
            {
                offset.x += textFont->GetOffsetTab() * textScale.x;
                continue;
            }
            else if (glyphChar == '\n')
            {
                offset.x = 0.0f;
                offset.y -= textFont->GetOffsetNewline() * textScale.y;
                continue;
            }
            else if (glyphChar == ' ')
            {
                offset.x += textFont->GetOffsetSpace() * textScale.x;
                continue;
            }

            const auto alphabetEntry = alphabet.find(glyphChar);
            if (alphabetEntry == alphabet.end())
                continue;
            const sGlyph& glyph = alphabetEntry->second;

            sTextInstance t;
            t._info.x = textPosition.x + offset.x;
            t._info.y = textPosition.y + (offset.y - (float)((glyph._height - glyph._top) * textScale.y));
            t._info.z = (float)glyph._width * textScale.x;
            t._info.w = (float)glyph._height * textScale.y;
            t._atlasInfo.x = (float)glyph._atlasXOffset / (float)atlas->GetWidth();
            t._atlasInfo.y = (float)glyph._atlasYOffset / (float)atlas->GetHeight();
            t._atlasInfo.z = (float)glyph._width / (float)atlas->GetWidth();
            t._atlasInfo.w = (float)glyph._height / (float)atlas->GetHeight();

            offset.x += glyph._advanceX * textScale.x;

            memcpy((void*)((usize)_textInstances + _textInstancesByteSize), &t, sizeof(sTextInstance));
            _textInstancesByteSize += sizeof(sTextInstance);

            actualCharCount += 1;
        }

        const cMaterialInstance mi(0, it.GetMaterial());
        memcpy(_textMaterials, &mi, sizeof(cMaterialInstance));
        _textMaterialsByteSize += sizeof(cMaterialInstance);

        _gfx->WriteBuffer(_textInstanceBuffer, 0, _textInstancesByteSize, _textInstances);
        _gfx->WriteBuffer(_textMaterialBuffer, 0, _textMaterialsByteSize, _textMaterials);

        _gfx->BindRenderPass(_text);
        _gfx->BindTexture(_text->GetShader(), "FontAtlas", atlas, 0);
        _gfx->DrawQuads(actualCharCount);
        _gfx->UnbindRenderPass(_text);
    }
}*/