// thread_guard.hpp

#include <thread>
#include <atomic>
#include <cassert>

namespace triton
{
	class CThreadGuard final
	{
		static std::atomic<std::thread::id> _mainThreadId;
		static std::atomic<std::thread::id> _renderThreadId;

	public:
		static void CaptureMainThreadId()
		{
			_mainThreadId.store(std::this_thread::get_id());
		}

		static void CaptureRenderThreadId()
		{
			_renderThreadId.store(std::this_thread::get_id());
		}

		static void AssertMain()
		{
			assert(std::this_thread::get_id() == _mainThreadId.load());
		}

		static void AssertRender()
		{
			assert(std::this_thread::get_id() == _renderThreadId.load());
		}
	};
}