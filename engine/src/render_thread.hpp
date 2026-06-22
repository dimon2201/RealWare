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

		XEngineMTSynchronization* _synchronization = nullptr;
		XRenderSubsystem* _renderSubsystem = nullptr;
		std::atomic<types::boolean> _initialized = types::K_FALSE;
		std::atomic<types::boolean> _frameDone = types::K_FALSE;
		std::mutex _threadMutex;
		std::condition_variable _cv;

		void NotifyThread();
		void MakeContextCurrent(const CRenderFrame* renderFrame, iGraphicsContextBackend* contextBackend);
		void ExecuteCommands(const CRenderFrame* renderFrame, iGraphicsDrawcallBackend* drawcallBackend, iGraphicsResourceBackend* resourceBackend, cGraphics* gfx);
		void Present(const CRenderFrame* renderFrame, iGraphicsContextBackend* contextBackend);

	public:
		explicit cRenderThread(cContext* context, XEngineMTSynchronization* synchronization, XRenderSubsystem* renderSubsystem);
		virtual ~cRenderThread() = default;

		virtual void ThreadFunction() override;

		inline types::boolean IsInitialized() const
		{
			return _initialized.load();
		}

		inline types::boolean IsFrameDone() const
		{
			return _frameDone.load();
		}
	};
}