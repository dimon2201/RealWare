#include "render_pass.hpp"
#include "context.hpp"
#include "engine.hpp"
#include "instance_buffer.hpp"
#include "components.hpp"
#include "stack.hpp"
#include "application.hpp"
#include "render_subsystem.hpp"
#include "graphics.hpp"

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
        caps->maxRenderMaterialCount,
        1
    ));
    _materialBuffer = renderSubsystem->FetchResult<cBuffer*>();
    
    renderSubsystem->PushCommand(SRenderCommand(
        ERenderCommand::CREATE_BUFFER,
        (cpuword)cBuffer::eType::STORAGE,
        (cpuword)nullptr,
        caps->maxRenderTextureAtlasTextureCount,
        3
    ));
    _textureBuffer = renderSubsystem->FetchResult<cBuffer*>();
    
    sChunkAllocatorDescriptor cad = {};
    cad.chunkByteSize = caps->hashTableChunkByteSize;
    cad.maxChunkCount = caps->hashTableMaxChunkCount;
    cad.hashTableSize = caps->hashTableSize;
    _dirtyStaticInstances = _context->Create<cStack<SInstanceBufferHandle>>(_context, cad);
}

triton::XRenderPass::~XRenderPass()
{
    XRenderSubsystem* renderSubsystem = _context->GetSubsystem<XRenderSubsystem>();
    
    _context->Destroy<cStack<SInstanceBufferHandle>>(_dirtyStaticInstances);

    renderSubsystem->PushCommand(SRenderCommand(
        ERenderCommand::DESTROY_BUFFER,
        (cpuword)_textureBuffer
    ));
    renderSubsystem->PushCommand(SRenderCommand(
        ERenderCommand::DESTROY_BUFFER,
        (cpuword)_materialBuffer
    ));
}

void triton::XRenderPass::WriteDirtyStaticInstancesToGPU()
{
    CThreadGuard::AssertRender();

    while (!_dirtyStaticInstances->IsEmpty())
    {
        SInstanceBufferHandle handle = *_dirtyStaticInstances->Pop().data;
        _instanceBufferStatic->Write(handle);
    }
}

void triton::XRenderPass::WriteDynamicInstancesToGPU()
{
    CThreadGuard::AssertRender();

    _instanceBufferDynamic->WriteAll();
}

void triton::XRenderPass::WriteStaticInstanceToGPU(SInstanceBufferHandle& instance)
{
    _dirtyStaticInstances->Push(std::move(instance));
}

void triton::XRenderPass::Bind()
{
    CThreadGuard::AssertRender();

    iGraphicsPipelineBackend* gfxPipelineBackend = _context->GetSubsystem<iGraphicsPipelineBackend>();
    gfxPipelineBackend->BindVertexArray(_vertexArray->GetGPUVertexArray());
}

void triton::XRenderPass::SynchronizeWithGPU()
{
    CThreadGuard::AssertRender();

    WriteDirtyStaticInstancesToGPU();
    WriteDynamicInstancesToGPU();
}

void triton::XRenderPass::Execute()
{
    CThreadGuard::AssertRender();

    SynchronizeWithGPU();
    Bind();
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
    std::vector<SShaderDefine> defines = {};
    for (usize i = 0; i < textures.size(); i++)
    {
        const usize textureAtlasTextureIndex = i;
        const std::string& textureAtlasTextureName = textures[i]._name;
        defines.push_back({ textureAtlasTextureName, textureAtlasTextureIndex });
    }

    return defines;
}