// render_thread.cpp

#include "render_thread.hpp"
#include "context.hpp"
#include "engine.hpp"
#include "input.hpp"
#include "texture_manager.hpp"
#include "graphics.hpp"
#include "graphics_context_backend.hpp"
#include "render_subsystem.hpp"
#include "thread_guard.hpp"

using namespace types;

triton::cRenderThread::cRenderThread(cContext* context, XFrameSync* sync, XRenderSubsystem* renderSubsystem) : cThread(context), _sync(sync), _renderSubsystem(renderSubsystem)
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
		gfxContextBackend->MakeWindowGraphicsContextCurrent(windows->At(i)->GetBackendWindow());
		gfxContextBackend->CreateGraphicsContext();
	}

	_initialized.store(K_TRUE);
	_renderSubsystem->NotifyMainThread();

	// Initialize subsystems
	_context->GetSubsystem<cTextureAtlas>()->Initialize(cVector3(1024, 1024, 16));
	_context->GetSubsystem<cGraphics>()->Initialize();

	cGraphics* gfx = _context->GetSubsystem<cGraphics>();
	iGraphicsDrawcallBackend* gfxDrawcallBackend = _context->GetBackend<iGraphicsDrawcallBackend>();

	while (K_TRUE)
	{
		_sync->WaitRenderThread(_cv);
		
		if (_sync->CheckFrameSwapChain())
			break;

		{
			std::lock_guard<std::mutex> lock(_sync->_mutex);
			std::cout << _sync->_renderThreadSwapChainSnapshot._stopSync << std::endl;
		}

		const CRenderFrame* renderFrame = _sync->AcquireFrame();

		MakeContextCurrent(renderFrame, gfxContextBackend);
		ExecuteCommands(renderFrame, gfxDrawcallBackend, gfx);
		Present(renderFrame, gfxContextBackend);

		_sync->FreeFrame(renderFrame->GetIndexInSwapChain());

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

void triton::cRenderThread::ExecuteCommands(const CRenderFrame* renderFrame, iGraphicsDrawcallBackend* drawcallBackend, cGraphics* gfx)
{
	while (auto result = renderFrame->Next())
	{
		const SRenderCommand* cmd = *result;

		switch (cmd->_command)
		{
		case ERenderCommand::RESIZE_RENDER_TARGETS:
			gfx->ResizeRenderTargets(glm::vec2(
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
		}
	}
}

void triton::cRenderThread::Present(const CRenderFrame* renderFrame, iGraphicsContextBackend* contextBackend)
{
	contextBackend->SwapWindowBuffers(renderFrame->GetWindow()->GetBackendWindow());
}