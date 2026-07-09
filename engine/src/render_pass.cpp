#include "render_pass.hpp"
#include "context.hpp"
#include "engine.hpp"
#include "instance_buffer.hpp"
#include "components.hpp"
#include "stack.hpp"
#include "application.hpp"
#include "render_subsystem.hpp"
#include "graphics.hpp"
#include "buffer_view.hpp"
#include "camera.hpp"
#include "input.hpp"

using namespace triton::ecs::components;
using namespace types;

triton::XRenderPass::XRenderPass(cContext* context) : iObject(context)
{
    XRenderSubsystem* renderSubsystem = _context->GetSubsystem<XRenderSubsystem>();
    IApplication* app = _context->GetSubsystem<cEngine>()->GetApplication();
    const sCapabilities* caps = app->GetCapabilities();
    renderSubsystem->PushCommand(SRenderCommand(
        ERenderCommand::CREATE_BUFFER,
        (cpuword)cBuffer::eType::STORAGE,
        (cpuword)nullptr,
        caps->maxRenderTextureAtlasTextureCount,
        3
    ));
    _textureBuffer = renderSubsystem->FetchResult<cBuffer*>();
}

triton::XRenderPass::~XRenderPass()
{
    XRenderSubsystem* renderSubsystem = _context->GetSubsystem<XRenderSubsystem>();
    renderSubsystem->PushCommand(SRenderCommand(
        ERenderCommand::DESTROY_BUFFER,
        (cpuword)_textureBuffer
    ));
}

void triton::XRenderPass::Bind()
{
    CThreadGuard::AssertRender();

    CGPUShader shader = _shader->GetGPUShader();
    iGraphicsPipelineBackend* gfxPipelineBackend = _context->GetBackend<iGraphicsPipelineBackend>();
    gfxPipelineBackend->BindShader(&shader);
    gfxPipelineBackend->Viewport(_viewport);
    gfxPipelineBackend->BindDepthMode(_depthState);
    gfxPipelineBackend->BindBlendMode(_blendState);
    for (auto& tex : _inputTextures)
        gfxPipelineBackend->BindTextureNamed(&shader, tex._texture, tex._name, -1);
    if (_vertexArray)
        gfxPipelineBackend->BindVertexArray(_vertexArray->GetGPUVertexArray());
    if (_renderTarget)
        gfxPipelineBackend->BindRenderTarget(_renderTarget);
}

void triton::XRenderPass::Unbind()
{
    CThreadGuard::AssertRender();

    CGPUShader shader = _shader->GetGPUShader();
    iGraphicsPipelineBackend* gfxPipelineBackend = _context->GetBackend<iGraphicsPipelineBackend>();
    iGraphicsResourceBackend* gfxResourceBackend = _context->GetBackend<iGraphicsResourceBackend>();
    gfxPipelineBackend->UnbindShader();
    // TODO: uncomment this after debug
    //for (auto& tex : _inputTextures)
    //    gfxResourceBackend->UnbindTexture(tex._texture);
    if (_vertexArray)
        gfxPipelineBackend->UnbindVertexArray();
    if (_renderTarget)
        gfxPipelineBackend->UnbindRenderTarget();
}

void triton::XRenderPass::Draw()
{
    CThreadGuard::AssertRender();

    cGraphics* gfx = _context->GetSubsystem<cGraphics>();
    iGraphicsDrawcallBackend* gfxDrawcallBackend = _context->GetBackend<iGraphicsDrawcallBackend>();
    iGraphicsPipelineBackend* gfxPipelineBackend = _context->GetBackend<iGraphicsPipelineBackend>();

    switch (_dispatch)
    {
        case ERenderPassDispatch::GEOMETRY:
        {
            SBufferView<XRenderBatch> batchBuffer = _context->GetSubsystem<cGraphics>()->GetBatches();
            for (usize i = 0; i < batchBuffer._elementCount; i++)
            {
                const XRenderBatch& batch = batchBuffer._elements[i];
                const SGeometryView geometry = batch.GetGeometry();
                XCamera* camera = gfx->GetCamera(_camera);
                CGPUShader shader = _shader->GetGPUShader();

                // TODO: remove this line
                cInputWindow* ibw = _context->GetSubsystem<cInput>()->GetWindows()->At(0).data;
                cVector2 cursorPos = _context->GetSubsystem<cInput>()->GetCursorPosition(ibw);
                camera->Update(cursorPos, 800, 600, 65.0f, 0.01f, 10000.0f, 0.1f);
                ///////////////////////////////////////////////////////////////////////////

                camera->Bind(this);

                gfxDrawcallBackend->ClearColor(cVector4(0.45f, 0.0f, 0.8f, 1.0f));
                gfxDrawcallBackend->ClearDepth(1.0f);

                // Static
                static u32 time = 0;
                time += 1;
				// TODO: remove this
                cVector4 cameraWorldPos = cVector4(camera->_worldPosition.GetX(), camera->_worldPosition.GetY(), camera->_worldPosition.GetZ(), 0.0f);
                gfxPipelineBackend->SetShaderUniform(&shader, "UniformTime", (u32)time);
                gfxPipelineBackend->SetShaderUniform(&shader, "CameraPosWorldSpace", 1, (f32*)&cameraWorldPos);
                gfxPipelineBackend->SetShaderUniform(&shader, "InstanceBatchType", (u32)0);
                gfxPipelineBackend->SetShaderUniform(&shader, "InstanceOffset", (u32)batch.GetInstanceOffset(SRenderInstance::EUsage::STATIC));
                gfxDrawcallBackend->Draw(
                    geometry._indexCount,
                    geometry._vertexElementOffset,
                    geometry._indexElementOffset,
                    batch.GetInstanceCount(SRenderInstance::EUsage::STATIC)
                );

                // Dynamic
                /*gfxPipelineBackend->SetShaderUniform(&shader, "InstanceBatchType", 1);
                gfxPipelineBackend->SetShaderUniform(&shader, "InstanceOffset", (u32)batch.GetInstanceOffset(SRenderInstance::EUsage::DYNAMIC));
                gfxDrawcallBackend->Draw(
                    geometry._indexCount,
                    geometry._vertexElementOffset,
                    geometry._indexElementOffset,
                    batch.GetInstanceCount(SRenderInstance::EUsage::DYNAMIC)
                );*/
            }
            break;
        }
        case ERenderPassDispatch::PROCESSING:
        {
            gfxDrawcallBackend->DrawQuad();
            break;
        }
    }
}

void triton::XRenderPass::Execute()
{
    CThreadGuard::AssertRender();

    Bind();
    Draw();
    Unbind();
}

void triton::XRenderPass::ResizeViewport(const cVector2& size)
{
    _viewport.rect.SetZ(size.GetX());
    _viewport.rect.SetW(size.GetY());
}

void triton::XRenderPass::ResizeColorAttachments(const cVector2& size)
{
    XRenderSubsystem* renderSubsystem = _context->GetSubsystem<XRenderSubsystem>();
    renderSubsystem->PushCommand(SRenderCommand(
        ERenderCommand::RESIZE_RENDER_TARGET_COLORS,
        (cpuword)_renderTarget,
        size.GetX(),
        size.GetY()
    ));
}

void triton::XRenderPass::ResizeDepthAttachment(const cVector2& size)
{
    XRenderSubsystem* renderSubsystem = _context->GetSubsystem<XRenderSubsystem>();
    renderSubsystem->PushCommand(SRenderCommand(
        ERenderCommand::RESIZE_RENDER_TARGET_DEPTH,
        (cpuword)_renderTarget,
        size.GetX(),
        size.GetY()
    ));
}

triton::SShaderDefine triton::XRenderPass::SetInputTexture(types::usize slot, const SRenderPassTexture& texture)
{
    if (slot >= _inputTextures.size())
        return SShaderDefine("", 0);

    _inputTextures.at(slot) = texture;

    return SShaderDefine(texture._name, slot);
}

std::vector<triton::SShaderDefine> triton::XRenderPass::SetInputTextures(const std::vector<triton::SRenderPassTexture>& textures)
{
    _inputTextures = textures;

    std::vector<SShaderDefine> defines = {};
    for (usize i = 0; i < textures.size(); i++)
    {
        const usize textureAtlasTextureIndex = i;
        const std::string& textureAtlasTextureName = textures[i]._name;
        defines.push_back({ textureAtlasTextureName, textureAtlasTextureIndex });
    }

    return defines;
}