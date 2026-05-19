// frame_render_susbsytem.cpp

#include "frame_render_subsystem.hpp"
#include "context.hpp"
#include "graphics.hpp"
#include "time.hpp"
#include "render_thread.hpp"
#include "application.hpp"
#include "input_backend.hpp"
#include "input.hpp"

using namespace types;

triton::XFrameRenderSubsystem::XFrameRenderSubsystem(cContext* context) : iObject(context)
{
	_frontIndex = 0; // render thread reads it
	_backIndex = 1; // main thread writes it
	_frameReady.store(K_FALSE);
}

void triton::XFrameRenderSubsystem::Initialize()
{
	_frontIndex = 0; // render thread reads it
	_backIndex = 1; // main thread writes it
	_frameReady.store(K_FALSE);

	// Create render thread
	_renderThreadStateBuffer = (cRenderThreadState*)_context->GetMemoryAllocator()->Allocate(sizeof(cRenderThreadState) * 2, 64);
	{
		std::unique_lock<std::mutex> lock(_threadMutex);
		_renderThread = _context->Create<cRenderThread>(_context, this);
		_renderThread->Run();

		// Wait until render thread gets initialized to continue main thread
		_cv.wait(lock, [this] { return _renderThread->IsInitialized(); });
	}
}

void triton::XFrameRenderSubsystem::Shutdown()
{
	_context->GetMemoryAllocator()->Deallocate(_renderThreadStateBuffer);
	_context->Destroy<cRenderThread>(_renderThread);
}

void triton::XFrameRenderSubsystem::MainThreadFunction(IApplication* app)
{
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
	s32 windowCount = windows->GetSize();
	while (K_TRUE)
	{
		inputBackend->PollEvents();

		s32 windowCount = windows->GetSize();
		if (windowCount == 0)
			break;

		cRenderThreadState& renderThreadState = _renderThreadStateBuffer[_backIndex];
		renderThreadState.Reset();

		// Prepare frame for render thread
		for (s32 i = windowCount - 1; i > -1; i--)
		{
			cInputWindow* window = windows->At(i);
			if (window->GetRunState() == cInputWindow::eRunState::OPENED)
			{
				// Fill render commands for render thread
				renderThreadState.PushWindow(window);
				sRenderCommandArgs args;
				renderThreadState.PushCommand(eRenderCommand::CLEAR_SCREEN, std::move(args));
			}
			// Destroy window if needed
			else if (window->GetRunState() == cInputWindow::eRunState::CLOSED)
			{
				input->DestroyWindow(window);
				windows->Erase(i);
			}
		}

		// Publish frame
		{
			std::lock_guard<std::mutex> lock(_threadMutex);
			std::swap(_frontIndex, _backIndex);
			_frameReady.store(K_TRUE);
		}
		_renderThread->NotifyThread();
	}

	time->EndFrame();

	// Stop render thread
	_renderThread->Stop();

	app->Stop();
}

void triton::XFrameRenderSubsystem::NotifyMainThread()
{
	_cv.notify_one();
}