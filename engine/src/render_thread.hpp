// render_thread.hpp

#pragma once

#include <atomic>
#include "thread_subsystem.hpp"
#include "render_frame.hpp"

namespace triton
{
	class XRenderSubsystem;
	class cInputWindow;
	
	class cRenderThread final : public cThread
	{
		TRITON_OBJECT(cRenderThread)

		XRenderSubsystem* _renderSubsystem = nullptr;
		std::atomic<types::boolean> _initialized = types::K_FALSE;
		std::mutex _threadMutex;
		std::condition_variable _cv;

	public:
		explicit cRenderThread(cContext* context, XRenderSubsystem* frameRenderSubsystem);
		virtual ~cRenderThread() = default;

		virtual void ThreadFunction() override;
		void NotifyThread();
		void ExecuteRenderCommands(const CRenderFrame& renderFrame);

		inline types::boolean IsInitialized() const
		{
			return _initialized.load();
		}
	};
}