// engine.hpp

#pragma once

#include <thread>
#include <unordered_map>
#include <mutex>
#include <condition_variable>
#include "object.hpp"
#include "subsystem.hpp"
#include "types.hpp"

namespace triton
{
	struct sCapabilities;
	class cContext;
	class IApplication;
	template <typename T>
	class XDynamicArray;
	class XSynchronization;
	class XRenderCommandRecorder;
	class cRenderThread;

	inline std::thread::id gMainThreadId = std::this_thread::get_id();

	class CEngine final : public CSubsystem
	{
		TRITON_CLASS_NAME(CEngine)

		IApplication* _app = nullptr;
		XSynchronization* _sync = nullptr;
		XRenderCommandRecorder* _cmdRecorder = nullptr;
		cRenderThread* _renderThread = nullptr;

	public:
		explicit CEngine(cContext* context, IApplication* app);
		~CEngine();

		void Initialize();
		void Shutdown();
		void Run();

		inline IApplication* GetApplication() const { return _app; }

		inline XRenderCommandRecorder* GetRenderCommandRecorder() { return _cmdRecorder; }

		inline XSynchronization* GetSynchronization() { return _sync; }

	private:
		void InitializeSynchronization();
		void InitializeRenderCommandRecorder();
		void InitializeRenderThread();
		void ReleaseSynchronization();
		void ReleaseRenderCommandRecorder();
		void ReleaseRenderThread();
		void MainThreadFunction();
	};
}