// render_target.cpp

#include "render_target.hpp"
#include "thread_guard.hpp"
#include "context.hpp"
#include "engine.hpp"
#include "synchronization.hpp"

using namespace types;

triton::XRenderTarget::XRenderTarget(
	cContext* context,
    s32 poolIndex,
    const std::vector<XTexture::THandle>& colorAttachments,
    const XTexture::THandle& depthAttachment
) : iObject(context, poolIndex), _colorAttachments(colorAttachments), _depthAttachment(depthAttachment)
{
    CThreadGuard::AssertMain();

    _context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::CREATE_RENDER_TARGET,
        1,
        (cpuword)colorAttachments.data(),
        (cpuword)&depthAttachment
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