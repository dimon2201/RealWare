#include "render_pass.hpp"

triton::XRenderPass::XRenderPass(cContext* context, const SRenderPassDescriptor& desc, XRenderPassGPU* renderPassGPU)
    : iObject(context), _desc(desc), _renderPass(renderPass) {}

void triton::cRenderPass::ResizeViewport(const cVector2& size)
{
    _desc.viewport.rect.SetZ(size.x);
    _desc.viewport.rect.SetW(size.y);
}

void triton::cRenderPass::ResizeColorAttachments(const cVector2& size)
{
    iGraphicsPipelineBackend* gfxPipelineBackend = _context->GetBackend<iGraphicsPipelineBackend>();
    gfxPipelineBackend->ResizeRenderTargetColors(GetRenderTarget(), size);
}

void triton::cRenderPass::ResizeDepthAttachment(const cVector2& size)
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