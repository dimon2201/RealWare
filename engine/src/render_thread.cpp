// render_thread.cpp

#include "render_thread.hpp"
#include "context.hpp"
#include "engine.hpp"
#include "input.hpp"
#include "texture_manager.hpp"
#include "graphics.hpp"
#include "graphics_context_backend.hpp"

using namespace types;

void triton::cRenderThreadState::Reset()
{
	_windowCount = 0;
	_commandCount = 0;
}

void triton::cRenderThreadState::PushWindow(cInputWindow* window)
{
	_windows[_windowCount] = window;
	_windowCount += 1;
}

void triton::cRenderThreadState::PushCommand(eRenderCommand command, sRenderCommandArgs&& args)
{
	_commands[_commandCount] = command;
	_commandArgs[_commandCount] = args;
	_commandCount += 1;
}

triton::cRenderThread::cRenderThread(cContext* context, CEngineMultithreadedExecution* execution) : cThread(context), _execution(execution)
{
	_initialized.store(K_FALSE);
}

void triton::cRenderThread::ThreadFunction()
{
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
	_execution->NotifyMainThread();

	// Initialize subsystems
	_context->GetSubsystem<cTextureAtlas>()->Initialize(cVector3(1024, 1024, 16));
	_context->GetSubsystem<cGraphics>()->Initialize();

	while (K_TRUE)
	{
        u32 frontIndex;

        {
            std::unique_lock<std::mutex> lock(_threadMutex);
            _cv.wait(lock, [this] {
                return _execution->IsFrameReady();
            });
            frontIndex = _execution->GetFrontIndex();
			_execution->MarkFrameReady(K_FALSE);
        }

        const cRenderThreadState& renderThreadState = _execution->GetRenderThreadStateBuffer()[frontIndex];
		ExecuteFrameCommands(renderThreadState);
	}

	// Initialize graphics-related subsystems
	// NOTE: order matters
	//_context->GetSubsystem<cTextureAtlas>()->Initialize(cVector3(2048, 2048, 16));
	//_context->GetSubsystem<cGraphics>()->Initialize();
}

void triton::cRenderThread::NotifyThread()
{
	_cv.notify_one();
}

void triton::cRenderThread::ExecuteFrameCommands(const cRenderThreadState& renderThreadState)
{
}