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
	const sCapabilities* caps = _context->GetSubsystem<cEngine>()->GetCapabilities();
	_resultBuffer = (u8*)_context->GetMemoryAllocator()->Allocate(caps->futureResultBufferByteSize, 64);
	_initialized.store(K_FALSE);
}

triton::cRenderThread::~cRenderThread()
{
	_context->GetMemoryAllocator()->Deallocate(_resultBuffer);
}

void triton::cRenderThread::ThreadFunction()
{
	std::mutex mtx;
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

	{
		std::lock_guard<std::mutex> lg(mtx);
		std::cout << "Render thread initialized\n\n";
	}


	cGraphics* gfx = _context->GetSubsystem<cGraphics>();
	iGraphicsDrawcallBackend* gfxDrawcallBackend = _context->GetBackend<iGraphicsDrawcallBackend>();
	iGraphicsResourceBackend* gfxResourceBackend = _context->GetBackend<iGraphicsResourceBackend>();

	while (K_TRUE)
	{
		_synchronization->LoopStart();
		_renderSubsystem->NotifyMainThread();

		{
			std::lock_guard<std::mutex> lg(mtx);
			if (_synchronization->_renderThreadSwapChainSnapshot._frames[0] == EFrameState::EXECUTE_FULL)
				std::cout << "Full\n";
			if (_synchronization->_renderThreadSwapChainSnapshot._frames[0] == EFrameState::EXECUTE_COMMANDS)
				std::cout << "Commands\n";
			if (_synchronization->_renderThreadSwapChainSnapshot._frames[1] == EFrameState::EXECUTE_FULL)
				std::cout << "Full\n";
			if (_synchronization->_renderThreadSwapChainSnapshot._frames[1] == EFrameState::EXECUTE_COMMANDS)
				std::cout << "Commands\n\n";
		}

		_synchronization->WaitForProducedFrame(_cv);

		{ std::lock_guard<std::mutex> lg(mtx); std::cout << "Render thread renders\n"; }
		
		if (!_synchronization->IsAlive())
			break;

		{ std::lock_guard<std::mutex> lg(mtx); std::cout << "A\n"; }

		const CRenderFrame* renderFrame = _synchronization->AcquireProducedFrame();
		EFrameState renderFrameState = renderFrame->GetState();

		{ std::lock_guard<std::mutex> lg(mtx); std::cout << "B\n"; }

		if (renderFrameState == EFrameState::EXECUTE_FULL)
		{
			// Full job

			// Execute render passes
			gfx->ExecuteDefaultRenderPasses();

			// Core events
			MakeContextCurrent(renderFrame, gfxContextBackend);
			ExecuteCommands(renderFrame, gfxDrawcallBackend, gfxResourceBackend, gfx);
			Present(renderFrame, gfxContextBackend);
		}
		else if (renderFrameState == EFrameState::EXECUTE_COMMANDS)
		{
			// Execute render commands only
			MakeContextCurrent(renderFrame, gfxContextBackend);
			ExecuteCommands(renderFrame, gfxDrawcallBackend, gfxResourceBackend, gfx);
		}

		{ std::lock_guard<std::mutex> lg(mtx); std::cout << "C\n"; }

		_synchronization->ReleaseFrame(renderFrame->GetIndexInSwapChain());

		{ std::lock_guard<std::mutex> lg(mtx); std::cout << "D\n"; }

		_synchronization->LoopFinish();

		{ std::lock_guard<std::mutex> lg(mtx); std::cout << "E\n"; }

		_renderSubsystem->NotifyMainThread();

		{ std::lock_guard<std::mutex> lg(mtx); std::cout << "F\n"; }
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
		case ERenderCommand::CREATE_TEXTURE:
			cTexture* result = resourceBackend->CreateTexture(
				cVector3(cmd->_args._argA, cmd->_args._argB, cmd->_args._argC),
				(cTexture::eDimension)cmd->_args._argD,
				(cTexture::eFormat)cmd->_args._argE,
				(u8*)cmd->_args._argF,
				cmd->_args._argG
			);
			memcpy(&_resultBuffer[0], &result, sizeof(cTexture*));
			cTexture* t = (cTexture*)&_resultBuffer[0];
			break;
		}
	}
}

void triton::cRenderThread::Present(const CRenderFrame* renderFrame, iGraphicsContextBackend* contextBackend)
{
	contextBackend->SwapWindowBuffers(renderFrame->GetWindow()->GetBackendWindow());
}