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

	iGraphicsDrawcallBackend* gfxDrawcallBackend = _context->GetBackend<iGraphicsDrawcallBackend>();

	while (K_TRUE)
	{
        u32 frontIndex;

        {
            std::unique_lock<std::mutex> lock(_sync->_mutex);
            _cv.wait(lock, [this] {
                return _sync->_state == EState::READY;
            });
            //frontIndex = _renderSubsystem->GetFrontIndex();
        }

        //CRenderFrame& renderFrame = _renderSubsystem->GetFrameSwapChain()->_frames[frontIndex];

		//MakeContextCurrent(renderFrame, gfxContextBackend);
		//ExecuteCommands(renderFrame, gfxDrawcallBackend);
		//Present(renderFrame, gfxContextBackend);

		{
			std::lock_guard<std::mutex> lock(_sync->_mutex);
			_sync->_state = EState::CONSUMED;
		}

		_renderSubsystem->NotifyMainThread();
	}
}

void triton::cRenderThread::NotifyThread()
{
	_cv.notify_one();
}

void triton::cRenderThread::MakeContextCurrent(const CRenderFrame& renderFrame, iGraphicsContextBackend* contextBackend)
{
	contextBackend->MakeWindowGraphicsContextCurrent(renderFrame.GetWindow()->GetBackendWindow());
}

void triton::cRenderThread::ExecuteCommands(CRenderFrame& renderFrame, iGraphicsDrawcallBackend* drawcallBackend)
{
	while (auto command = renderFrame.Pop())
	{
		switch (command->_command)
		{
		case ERenderCommand::CLEAR:
			drawcallBackend->ClearColor(cVector4(
				command->_args._argA,
				command->_args._argB,
				command->_args._argC,
				command->_args._argD
			));
			break;
		}
	}
}

void triton::cRenderThread::Present(const CRenderFrame& renderFrame, iGraphicsContextBackend* contextBackend)
{
	contextBackend->SwapWindowBuffers(renderFrame.GetWindow()->GetBackendWindow());
}