// engine.hpp

#pragma once

#include <thread>
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
	class XDynamicArray;
	class XSynchronization;
	class XRenderCommandRecorder;
	class cRenderThread;

	inline std::thread::id gMainThreadId = std::this_thread::get_id();

	class cEngine final : public iObject
	{
		TRITON_OBJECT(cEngine)

		IApplication* _app = nullptr;
		const sCapabilities* _caps = nullptr;
		XSynchronization* _sync = nullptr;
		XRenderCommandRecorder* _cmdRecorder = nullptr;
		cRenderThread* _renderThread = nullptr;

	public:
		explicit cEngine(cContext* context, IApplication* app);
		virtual ~cEngine() override final;

		void Initialize();
		void Shutdown();
		void Run();

		inline IApplication* GetApplication() const { return _app; }
		inline const sCapabilities* GetCapabilities() const { return _caps; }

		inline XRenderCommandRecorder* GetRenderCommandRecorder()
		{
			return _cmdRecorder;
		}

		inline XSynchronization* GetSynchronization()
		{
			return _sync;
		}

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