// render_susbsytem.hpp

#include <mutex>
#include <condition_variable>
#include "object.hpp"
#include "types.hpp"
#include "render_frame.hpp"
#include "assert_thread.hpp"

namespace triton
{
	class IApplication;
	class cRenderThread;
	class CRenderFrame;

	class XRenderSubsystem final : public iObject
	{
		TRITON_OBJECT(XRenderSubsystem)

		CRenderFrame* _frameBuffer = nullptr;
		cRenderThread* _renderThread = nullptr;
		types::u32 _frontIndex = 0;
		types::u32 _backIndex = 0;
		std::atomic<types::boolean> _frameReady;
		std::mutex _threadMutex;
		std::condition_variable _cv;
		std::queue<SRenderCommand> _externalCommands;

	public:
		explicit XRenderSubsystem(cContext* context);
		virtual ~XRenderSubsystem() = default;

		void Initialize();
		void Shutdown();
		void MainThreadFunction(IApplication* app);
		void NotifyMainThread();
		void PushCommand(const SRenderCommand& command);

		inline types::boolean IsFrameReady() const
		{
			CThreadGuard::AssertRender();

			return _frameReady.load();
		}

		inline types::u32 GetFrontIndex() const
		{
			CThreadGuard::AssertRender();

			return _frontIndex;
		}

		inline types::u32 GetBackIndex() const
		{
			CThreadGuard::AssertRender();

			return _backIndex;
		}

		inline void MarkFrameReady(types::boolean value)
		{
			CThreadGuard::AssertRender();

			_frameReady.store(value);
		}

		inline CRenderFrame* GetRenderFrameBuffer() const
		{
			CThreadGuard::AssertRender();

			return _frameBuffer;
		}
	};
}