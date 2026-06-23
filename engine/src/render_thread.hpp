// render_thread.hpp

#pragma once

#include <atomic>
#include <cassert>
#include <thread>
#include "thread_subsystem.hpp"
#include "render_frame.hpp"

namespace triton
{
	class XRenderSubsystem;
	class iGraphicsContextBackend;
	class iGraphicsDrawcallBackend;
	class iGraphicsResourceBackend;
	class cInputWindow;
	class XEngineMTSynchronization;
	class cGraphics;

	class cRenderThread final : public cThread
	{
		TRITON_OBJECT(cRenderThread)

		types::u8* _resultBuffer = nullptr;
		XEngineMTSynchronization* _synchronization = nullptr;
		XRenderSubsystem* _renderSubsystem = nullptr;
		std::atomic<types::boolean> _initialized = types::K_FALSE;
		std::atomic<types::boolean> _isFrameFinished = types::K_FALSE;
		std::mutex _threadMutex;
		std::condition_variable _cv;

		void NotifyThread();
		void MakeContextCurrent(const CRenderFrame* renderFrame, iGraphicsContextBackend* contextBackend);
		void ExecuteCommands(const CRenderFrame* renderFrame, iGraphicsDrawcallBackend* drawcallBackend, iGraphicsResourceBackend* resourceBackend, cGraphics* gfx);
		void Present(const CRenderFrame* renderFrame, iGraphicsContextBackend* contextBackend);

	public:
		explicit cRenderThread(cContext* context, XEngineMTSynchronization* synchronization, XRenderSubsystem* renderSubsystem);
		~cRenderThread() override;

		virtual void ThreadFunction() override;

		template <typename TResult>
		TResult FetchCommandResult(std::condition_variable& cv)
		{
			CThreadGuard::AssertMain();

			_synchronization->WaitForFreeFrame(cv);
			_synchronization->ProduceFrame(EFrameState::EXECUTE_COMMANDS);
			_synchronization->WaitForFrameFinish(cv, this);

			TResult r = TResult(_context);
			memcpy(&r, &_resultBuffer[0], sizeof(TResult));

			return r;
		}

		inline types::boolean IsInitialized() const
		{
			return _initialized.load();
		}

		inline types::boolean IsFrameFinished() const
		{
			return _isFrameFinished.load();
		}
	};
}