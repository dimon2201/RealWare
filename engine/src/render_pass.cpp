#include "render_pass.hpp"
#include "context.hpp"
#include "engine.hpp"
#include "graphics_pipeline_backend.hpp"
#include "graphics_resource_backend.hpp"
#include "instance_buffer.hpp"
#include "components.hpp"

using namespace triton::ecs::components;
using namespace types;

triton::XRenderPass::XRenderPass(cContext* context, const SRenderPassDescriptor& desc, XRenderPassGPU* renderPassGPU)
    : iObject(context), _desc(desc), _renderPassGPU(renderPassGPU)
{
    const sCapabilities* caps = _context->GetSubsystem<cEngine>()->GetApplication()->GetCapabilities();
    sChunkAllocatorDescriptor cad = {};
    cad.chunkByteSize = caps->hashTableChunkByteSize;
    cad.maxChunkCount = caps->hashTableMaxChunkCount;
    cad.hashTableSize = caps->hashTableSize;
    _dirtyStaticInstances = _context->Create<cStack<SInstanceBufferHandle>>(_context, cad);
}

void triton::XRenderPass::WriteDirtyStaticInstancesToGPU()
{
    while (_dirtyStaticInstances->IsEmpty())
    {
        SInstanceBufferHandle handle = _dirtyStaticInstances->Pop();
    }
}

void triton::XRenderPass::WriteDynamicInstancesToGPU()
{
}

void triton::XRenderPass::WriteStaticInstanceToGPU(const SInstanceBufferHandle& instance)
{
    _dirtyStaticInstances->Push(std::move(instance));
}

void triton::XRenderPass::SynchronizeGPU()
{
}

void triton::XRenderPass::Execute()
{
}

void triton::XRenderPass::ResizeViewport(const cVector2& size)
{
    _desc._viewport.rect.SetZ(size.GetX());
    _desc._viewport.rect.SetW(size.GetY());
}

void triton::XRenderPass::ResizeColorAttachments(const cVector2& size)
{
    iGraphicsPipelineBackend* gfxPipelineBackend = _context->GetBackend<iGraphicsPipelineBackend>();
    gfxPipelineBackend->ResizeRenderTargetColors(GetRenderTarget(), glm::vec2(size.GetX(), size.GetY()));
}

void triton::XRenderPass::ResizeDepthAttachment(const cVector2& size)
{
    iGraphicsResourceBackend* gfxResourceBackend = _context->GetBackend<iGraphicsResourceBackend>();
    cRenderTarget* renderTarget = GetRenderTarget();
    renderTarget->SetDepthAttachment(
        gfxResourceBackend->ResizeTexture(
            renderTarget->GetDepthAttachment(),
            size
        )
    );
}

triton::cVertexArray* triton::XRenderPass::GetVertexArray() const
{
    return _renderPassGPU->GetVertexArray();
}

triton::cShader* triton::XRenderPass::GetShader() const
{
    return _renderPassGPU->GetShader();
}

triton::XInstanceBuffer* triton::XRenderPass::GetInstanceBuffer() const
{
    return _renderPassGPU->GetInstanceBuffer();
}

triton::cBuffer* triton::XRenderPass::GetMaterialBuffer() const
{
    return _renderPassGPU->GetMaterialBuffer();
}

triton::cBuffer* triton::XRenderPass::GetTextureBuffer() const
{
    return _renderPassGPU->GetTextureBuffer();
}