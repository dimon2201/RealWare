// render_target.cpp

#include "render_target.hpp"
#include "thread_guard.hpp"
#include "context.hpp"
#include "engine.hpp"
#include "synchronization.hpp"
#include "texture_pool.hpp"

using namespace types;

triton::XRenderTarget::XRenderTarget(
	cContext* context,
    s32 poolIndex,
    const std::vector<XTexture::THandle>& colorAttachments,
    const XTexture::THandle& depthAttachment
) : iObject(context, poolIndex), _colorAttachments(colorAttachments), _depthAttachment(depthAttachment)
{
    CThreadGuard::AssertMain();

    PTexturePool* texturePool = _context->GetPool<PTexturePool>();

    std::vector<CGPUTextureResource> gpuColorAttachments;
    for (usize i = 0; i < colorAttachments.size(); i++)
        gpuColorAttachments.push_back(texturePool->Get(colorAttachments[i]).value().get().GetGPUResource());

    const CGPUTextureResource gpuDepthAttachment = texturePool->Get(depthAttachment).value().get().GetGPUResource();

    _context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::CREATE_RENDER_TARGET,
        1,
        (cpuword)gpuColorAttachments.data(),
        (cpuword)&gpuDepthAttachment
    ));

    _gpuRenderTarget =
        _context->GetSubsystem<CEngine>()->
        GetSynchronization()->
        WaitForRenderCommandResult<CGPURenderTargetResource>();
}

triton::XRenderTarget::~XRenderTarget()
{
    CThreadGuard::AssertMain();

    _context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::DESTROY_RENDER_TARGET,
        (cpuword)&_gpuRenderTarget
    ));

    _context->GetSubsystem<CEngine>()->
        GetSynchronization()->
        WaitForRenderCommandResult<void*>();
}