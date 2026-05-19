// engine.hpp

#pragma once

#include <unordered_map>
#include <mutex>
#include <condition_variable>
#include "object.hpp"
#include "input_window.hpp"
#include "types.hpp"

namespace triton
{
	struct sCapabilities;
	class cContext;
	class IApplication;
	template <typename T>
	class cStack;
	class cRenderThread;
	class cRenderThreadState;

	class CEngineMultithreadedExecution : public iObject
	{
		TRITON_OBJECT(CEngineMultithreadedExecution)

		cRenderThreadState* _renderThreadStateBuffer = nullptr;
		types::u32 _frontIndex = 0;
		types::u32 _backIndex = 0;
		std::atomic<types::boolean> _frameReady;
		cRenderThread* _renderThread = nullptr;
		std::mutex _threadMutex;
		std::condition_variable _cv;

	public:
		explicit CEngineMultithreadedExecution(cContext* context);
		virtual ~CEngineMultithreadedExecution();

		void Run(IApplication* app);
		void NotifyMainThread();

		inline types::boolean IsFrameReady() const
		{
			return _frameReady.load();
		}

		inline types::u32 GetFrontIndex() const
		{
			return _frontIndex;
		}

		inline types::u32 GetBackIndex() const
		{
			return _backIndex;
		}

		inline void MarkFrameReady(types::boolean value)
		{
			_frameReady.store(value);
		}
		
		inline cRenderThreadState* GetRenderThreadStateBuffer() const
		{
			return _renderThreadStateBuffer;
		}
	};

	class cEngine final : public iObject
	{
		TRITON_OBJECT(cEngine)

		IApplication* _app = nullptr;
		const sCapabilities* _caps = nullptr;
		CEngineMultithreadedExecution* _execution = nullptr;

	public:
		explicit cEngine(cContext* context, IApplication* app);
		virtual ~cEngine() override final;

		void Initialize();
		void Shutdown();
		void Run();

		inline IApplication* GetApplication() const { return _app; }
		inline const sCapabilities* GetCapabilities() const { return _caps; }
	};
}