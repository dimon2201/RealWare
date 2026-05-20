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
	class cInputWindow;

	class cRenderThread final : public cThread
	{
		TRITON_OBJECT(cRenderThread)

		XRenderSubsystem* _renderSubsystem = nullptr;
		std::atomic<types::boolean> _initialized = types::K_FALSE;
		std::mutex _threadMutex;
		std::condition_variable _cv;

		void NotifyThread();
		void MakeContextCurrent(const CRenderFrame& renderFrame, iGraphicsContextBackend* contextBackend);
		void ExecuteCommands(CRenderFrame& renderFrame, iGraphicsDrawcallBackend* drawcallBackend);
		void Present(const CRenderFrame& renderFrame, iGraphicsContextBackend* contextBackend);

	public:
		explicit cRenderThread(cContext* context, XRenderSubsystem* renderSubsystem);
		virtual ~cRenderThread() = default;

		virtual void ThreadFunction() override;

		inline types::boolean IsInitialized() const
		{
			return _initialized.load();
		}
	};
}