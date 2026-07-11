// render_thread.hpp

#pragma once

#include <cstring>
#include <atomic>
#include <cassert>
#include <thread>
#include "thread_subsystem.hpp"
#include "render_frame.hpp"
#include "thread_guard.hpp"
#include "types.hpp"

namespace triton
{
	class XRenderSubsystem;
	class iGraphicsContextBackend;
	class iGraphicsDrawcallBackend;
	class iGraphicsResourceBackend;
	class iGraphicsPipelineBackend;
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
		std::mutex _threadMutex;
		std::condition_variable _cv;

		void NotifyThread();
		void MakeContextCurrent(const CRenderFrame* renderFrame, iGraphicsContextBackend* contextBackend);
		void ExecuteCommands(const CRenderFrame* renderFrame, iGraphicsDrawcallBackend* drawcallBackend, iGraphicsResourceBackend* resourceBackend, iGraphicsPipelineBackend* pipelineBackend, cGraphics* gfx);
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
			_renderSubsystem->ResetScratchFrame();
			NotifyThread();
			_synchronization->WaitForLoopFinish(cv);
			_synchronization->_mainThreadSwapChainSnapshot._isLoopFinished = types::K_FALSE;
			{
				std::lock_guard<std::mutex> lock(_synchronization->_mutex);
				_synchronization->_renderThreadSwapChainSnapshot._isLoopFinished = types::K_TRUE;
			}
			
			TResult* pr = (TResult*)&_resultBuffer[0];
			TResult r = *(TResult*)&_resultBuffer[0];
			//memcpy(&r, &_resultBuffer[0], sizeof(TResult));
			//r = *(TResult*)&_resultBuffer[0];

			return r;
		}

		inline types::boolean IsInitialized() const
		{
			return _initialized.load();
		}
	};
}
