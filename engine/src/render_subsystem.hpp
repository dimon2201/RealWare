// render_susbsytem.hpp

#include <mutex>
#include <condition_variable>
#include "object.hpp"
#include "types.hpp"
#include "render_frame.hpp"
#include "thread_guard.hpp"

namespace triton
{
	class IApplication;
	class cRenderThread;
	class CRenderFrame;

	class SFrameSwapChain
	{
	public:
		CRenderFrame _frames[2] = { CRenderFrame(nullptr),  CRenderFrame(nullptr) };
	};

	class XRenderSubsystem final : public iObject
	{
		TRITON_OBJECT(XRenderSubsystem)

		SFrameSwapChain _frameSwapChain = {};
		cRenderThread* _renderThread = nullptr;
		types::u32 _frontIndex = 0;
		types::u32 _backIndex = 0;
		types::boolean _frameReady;
		types::boolean _frameConsumed;
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

		inline types::boolean IsFrameReady()
		{
			CThreadGuard::AssertRender();

			types::boolean isReady;
			{
				std::lock_guard<std::mutex> lock(_threadMutex);
				isReady = _frameReady;
			}

			return isReady;
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

		inline void PublishFrame()
		{
			CThreadGuard::AssertMain();

			{
				std::lock_guard<std::mutex> lock(_threadMutex);
				_frameReady = types::K_TRUE;
				_frameConsumed = types::K_FALSE;
			}
		}

		inline void ConsumeFrame()
		{
			CThreadGuard::AssertRender();

			{
				std::lock_guard<std::mutex> lock(_threadMutex);
				_frameReady = types::K_FALSE;
				_frameConsumed = types::K_TRUE;
			}
		}

		inline SFrameSwapChain* GetFrameSwapChain()
		{
			CThreadGuard::AssertRender();

			return &_frameSwapChain;
		}
	};
}