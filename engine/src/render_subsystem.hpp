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

	enum class EState
	{
		READY = 0,
		CONSUMED
	};

	class XRenderSubsystem final : public iObject
	{
		TRITON_OBJECT(XRenderSubsystem)

		SFrameSwapChain _frameSwapChain = {};
		cRenderThread* _renderThread = nullptr;
		types::u32 _frontIndex = 0;
		types::u32 _backIndex = 0;
		EState _state = EState::CONSUMED;
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

		inline EState GetState()
		{
			CThreadGuard::AssertRender();

			EState state;
			{
				std::lock_guard<std::mutex> lock(_threadMutex);
				state = _state;
			}

			return state;
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

		inline void MarkFrameReady()
		{
			CThreadGuard::AssertMain();

			{
				std::lock_guard<std::mutex> lock(_threadMutex);
				_state = EState::READY;
			}
		}

		inline void MarkFrameConsumed()
		{
			CThreadGuard::AssertRender();

			{
				std::lock_guard<std::mutex> lock(_threadMutex);
				_state = EState::CONSUMED;
			}
		}

		inline SFrameSwapChain* GetFrameSwapChain()
		{
			CThreadGuard::AssertRender();

			return &_frameSwapChain;
		}
	};
}