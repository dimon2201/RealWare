// render_thread.hpp

#pragma once

#include <atomic>
#include "thread_subsystem.hpp"

namespace triton
{
	class cInputWindow;
	class CEngineMultithreadedExecution;
	
	enum class eRenderCommand
	{
		NONE = 0,
		CLEAR_SCREEN
	};

	struct sRenderCommandArgs
	{
		types::cpuword _argA = 0;
		types::cpuword _argB = 0;
		types::cpuword _argC = 0;
		types::cpuword _argD = 0;
	};

	class cRenderThreadState
	{
		types::usize _windowCount = 0;
		cInputWindow* _windows[8] = {};
		types::usize _commandCount = 0;
		eRenderCommand _commands[512] = {};
		sRenderCommandArgs _commandArgs[512] = {};

	public:
		void Reset();
		void PushWindow(cInputWindow* window);
		void PushCommand(eRenderCommand command, sRenderCommandArgs&& args);
	};

	class cRenderThread final : public cThread
	{
		TRITON_OBJECT(cRenderThread)

		CEngineMultithreadedExecution* _execution = nullptr;
		std::atomic<types::boolean> _initialized = types::K_FALSE;
		std::mutex _threadMutex;
		std::condition_variable _cv;

	public:
		explicit cRenderThread(cContext* context, CEngineMultithreadedExecution* execution);
		virtual ~cRenderThread() = default;

		virtual void ThreadFunction() override;
		void NotifyThread();
		void ExecuteFrameCommands(const cRenderThreadState& renderThreadState);

		inline types::boolean IsInitialized() const
		{
			return _initialized.load();
		}
	};
}