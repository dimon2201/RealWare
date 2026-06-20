// render_susbsytem.cpp

#include "render_subsystem.hpp"
#include "context.hpp"
#include "graphics.hpp"
#include "time.hpp"
#include "render_thread.hpp"
#include "application.hpp"
#include "input_backend.hpp"
#include "input.hpp"
#include "thread_guard.hpp"

using namespace types;

triton::XRenderSubsystem::XRenderSubsystem(cContext* context) : iObject(context)
{
	CThreadGuard::AssertMain();
}

void triton::XRenderSubsystem::Initialize()
{
	CThreadGuard::AssertMain();

	_sync = _context->Create<XFrameSync>(_context, this);

	for (usize i = 0; i < 2; i++)
	{
		_sync->_mainThreadSwapChainSnapshot._frames[i] = EFrameState::READY;
		_sync->_renderThreadSwapChainSnapshot._frames[i] = EFrameState::READY;
	}
	
	// Create render thread
	cInputWindow* window = _context->GetSubsystem<cInput>()->GetWindows()->At(0).data;
	for (usize i = 0; i < 2; i++)
		_sync->_swapChain._frames[i].Reset(window);

	_scratchFrame.Reset(window);
	
	{
		std::unique_lock<std::mutex> lock(_sync->_mutex);
		_renderThread = _context->Create<cRenderThread>(_context, _sync, this);
		_renderThread->Run();

		// Wait until render thread gets initialized to continue main thread
		_cv.wait(lock, [this] { return _renderThread->IsInitialized(); });
	}
}

void triton::XRenderSubsystem::Shutdown()
{
	CThreadGuard::AssertMain();

	_context->Destroy<cRenderThread>(_renderThread);
	_context->Destroy<XFrameSync>(_sync);
}

void triton::XRenderSubsystem::MainThreadFunction(IApplication* app)
{
	CThreadGuard::AssertMain();

	if (app == nullptr)
		return;

	app->Setup();

	auto gfx = _context->GetSubsystem<cGraphics>();
	//auto camera = _context->GetSubsystem<cCameraSystem>();
	auto time = _context->GetSubsystem<cTime>();
	//auto physics = _context->GetSubsystem<cPhysics>();

	time->BeginFrame();

	iInputBackend* inputBackend = _context->GetBackend<iInputBackend>();
	cInput* input = _context->GetSubsystem<cInput>();
	cStack<cInputWindow>* windows = input->GetWindows();
	cInputWindow* window = windows->At(0).data;
	s32 windowCount = windows->GetSize();
	while (K_TRUE)
	{
		inputBackend->PollEvents();

		_sync->WaitMainThread(_cv);

		s32 windowCount = windows->GetSize();
		if (windowCount == 0)
			break;

		// Prepare frame for render thread
		for (s32 i = windowCount - 1; i > -1; i--)
		{
			if (window->GetRunState() == cInputWindow::eRunState::OPENED)
			{
				// Fill frame
				SRenderCommand cmd;
				cmd._command = ERenderCommand::CLEAR;
				cmd._args._argA = (cpuword)1.0f;
				PushCommand(cmd);
			}
			// Destroy window if needed
			else if (window->GetRunState() == cInputWindow::eRunState::CLOSED)
			{
				// input->DestroyWindow(window);
				windows->Erase(i);

				Stop();
				_renderThread->NotifyThread();
				break;
			}
		}

		_sync->WriteFrame();
		_renderThread->NotifyThread();
	}

	input->DestroyWindow(window);

	time->EndFrame();

	// Stop render thread
	_renderThread->Stop();

	app->Stop();
}

void triton::XRenderSubsystem::NotifyMainThread()
{
	CThreadGuard::AssertRender();

	_cv.notify_one();
}

void triton::XRenderSubsystem::PushCommand(const SRenderCommand& command)
{
	CThreadGuard::AssertMain();

	_scratchFrame.PushCommand(command);
}

void triton::XRenderSubsystem::Stop()
{
	CThreadGuard::AssertMain();

	_sync->Stop();
}