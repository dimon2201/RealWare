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

	class XRenderSubsystem final : public iObject
	{
		TRITON_OBJECT(XRenderSubsystem)

	private:
		XFrameSync* _sync = nullptr;
		cRenderThread* _renderThread = nullptr;
		std::condition_variable _cv;
		CRenderFrame _externalFrame = CRenderFrame(nullptr);

	public:
		explicit XRenderSubsystem(cContext* context);
		virtual ~XRenderSubsystem() = default;

		void Initialize();
		void Shutdown();
		void MainThreadFunction(IApplication* app);
		void NotifyMainThread();
		void PushCommand(const SRenderCommand& command);
		void StopFrameExecution();

		inline CRenderFrame GetExternalFrame() const
		{
			return _externalFrame;
		}
	};
}