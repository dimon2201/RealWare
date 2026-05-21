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
	class cContext;

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

	class XFrameSync final : public iObject
	{
		TRITON_OBJECT(XFrameSync)

	public:
		SFrameSwapChain _frameSwapChain = {};
		types::u32 _frontIndex = 0;
		types::u32 _backIndex = 0;
		EState _state = EState::CONSUMED;
		std::mutex _mutex;

		explicit XFrameSync(cContext* context) : iObject(context) {}
		virtual ~XFrameSync() = default;
	};

	class XRenderSubsystem final : public iObject
	{
		TRITON_OBJECT(XRenderSubsystem)

		XFrameSync* _sync = nullptr;
		cRenderThread* _renderThread = nullptr;
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
	};
}