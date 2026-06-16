#include "render_pass.hpp"
#include "context.hpp"
#include "graphics_pipeline_backend.hpp"
#include "graphics_resource_backend.hpp"
#include "instance_buffer.hpp"
#include "components.hpp"

using namespace triton::ecs::components;
using namespace types;

triton::XRenderPass::XRenderPass(cContext* context, const SRenderPassDescriptor& desc, XRenderPassGPU* renderPassGPU)
    : iObject(context), _desc(desc), _renderPassGPU(renderPassGPU) {}

void triton::XRenderPass::UploadInstancesStatic(const SRenderData& data)
{
    _renderPassGPU->GetInstanceBuffer()->UploadStatic(data);
    _renderPassGPU->GetInstanceBuffer()->WriteToGPUStatic();
}

void triton::XRenderPass::UploadInstancesDynamic(const SRenderData& data)
{
    _renderPassGPU->GetInstanceBuffer()->UploadDynamic(data);
    _renderPassGPU->GetInstanceBuffer()->WriteToGPUDynamic();
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