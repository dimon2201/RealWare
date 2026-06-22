// render_thread.cpp

#include "render_thread.hpp"
#include "context.hpp"
#include "engine.hpp"
#include "input.hpp"
#include "texture_manager.hpp"
#include "graphics.hpp"
#include "graphics_context_backend.hpp"
#include "graphics_resource_backend.hpp"
#include "render_subsystem.hpp"
#include "thread_guard.hpp"
#include "stack.hpp"

using namespace types;

triton::cRenderThread::cRenderThread(cContext* context, XEngineMTSynchronization* synchronization, XRenderSubsystem* renderSubsystem) : cThread(context), _synchronization(synchronization), _renderSubsystem(renderSubsystem)
{
	_initialized.store(K_FALSE);
}

void triton::cRenderThread::ThreadFunction()
{
	CThreadGuard::CaptureRenderThreadId();

	// Create graphics contexts for windows
	cInput* inputSubsystem = _context->GetSubsystem<cInput>();
	cStack<cInputWindow>* windows = inputSubsystem->GetWindows();
	iGraphicsContextBackend* gfxContextBackend = _context->GetBackend<iGraphicsContextBackend>();
	for (usize i = 0; i < windows->GetSize(); i++)
	{
		gfxContextBackend->MakeWindowGraphicsContextCurrent(windows->At(i).data->GetBackendWindow());
		gfxContextBackend->CreateGraphicsContext();
	}

	_initialized.store(K_TRUE);
	_renderSubsystem->NotifyMainThread(); // TODO: move XRenderSubsystem::NotifyMainThread() to cRenderThread

	cGraphics* gfx = _context->GetSubsystem<cGraphics>();
	iGraphicsDrawcallBackend* gfxDrawcallBackend = _context->GetBackend<iGraphicsDrawcallBackend>();
	iGraphicsResourceBackend* gfxResourceBackend = _context->GetBackend<iGraphicsResourceBackend>();

	while (K_TRUE)
	{
		_frameDone.store(K_FALSE);

		_synchronization->WaitForProducedFrame(_cv);
		
		if (_synchronization->IsAlive())
			break;

		const CRenderFrame* renderFrame = _synchronization->AcquireProducedFrame();

		// Execute render passes
		gfx->ExecuteDefaultRenderPasses();

		// Core events
		MakeContextCurrent(renderFrame, gfxContextBackend);
		ExecuteCommands(renderFrame, gfxDrawcallBackend, gfxResourceBackend, gfx);
		Present(renderFrame, gfxContextBackend);

		_synchronization->ReleaseFrame(renderFrame->GetIndexInSwapChain());

		_frameDone.store(K_TRUE);

		_renderSubsystem->NotifyMainThread();
	}
}

void triton::cRenderThread::NotifyThread()
{
	_cv.notify_one();
}

void triton::cRenderThread::MakeContextCurrent(const CRenderFrame* renderFrame, iGraphicsContextBackend* contextBackend)
{
	contextBackend->MakeWindowGraphicsContextCurrent(renderFrame->GetWindow()->GetBackendWindow());
}

void triton::cRenderThread::ExecuteCommands(const CRenderFrame* renderFrame, iGraphicsDrawcallBackend* drawcallBackend, iGraphicsResourceBackend* resourceBackend, cGraphics* gfx)
{
	while (auto result = renderFrame->Next())
	{
		const SRenderCommand* cmd = *result;

		switch (cmd->_command)
		{
		case ERenderCommand::RESIZE_RENDER_TARGETS:
			gfx->ResizeRenderTargets(cVector2(
				cmd->_args._argA,
				cmd->_args._argB
			));
			break;
		case ERenderCommand::CLEAR:
			drawcallBackend->ClearColor(cVector4(
				cmd->_args._argA,
				cmd->_args._argB,
				cmd->_args._argC,
				cmd->_args._argD
			));
			break;
		case ERenderCommand::DRAW:
			drawcallBackend->Draw(
				cmd->_args._argA,
				cmd->_args._argB,
				cmd->_args._argC,
				cmd->_args._argD
			);
			break;
		case ERenderCommand::WRITE_BUFFER:
			resourceBackend->WriteBuffer(
				(cBuffer*)cmd->_args._argA,
				cmd->_args._argB,
				cmd->_args._argC,
				(const u8*)cmd->_args._argD
			);
			break;
		}
	}
}

void triton::cRenderThread::Present(const CRenderFrame* renderFrame, iGraphicsContextBackend* contextBackend)
{
	contextBackend->SwapWindowBuffers(renderFrame->GetWindow()->GetBackendWindow());
}