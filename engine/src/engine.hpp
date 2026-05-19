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
	class iApplication;
	template <typename T>
	class cStack;
	class cRenderThread;

	class cEngine final : public iObject
	{
		TRITON_OBJECT(cEngine)

		iApplication* _app = nullptr;
		const sCapabilities* _caps = nullptr;
		cRenderThread* _renderThread = nullptr;
		std::mutex _threadMutex;
		std::condition_variable _cv;

	public:
		explicit cEngine(cContext* context, iApplication* app);
		virtual ~cEngine() override final;

		void Initialize();
		void Shutdown();
		void Run();
		void NotifyThread();

		inline iApplication* GetApplication() const { return _app; }
		inline const sCapabilities* GetCapabilities() const { return _caps; }
	};
}