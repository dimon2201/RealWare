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
		XEngineMTSynchronization* _synchronization = nullptr;
		cRenderThread* _renderThread = nullptr;
		std::condition_variable _cv;
		CRenderFrame _scratchFrame = CRenderFrame(nullptr);

	public:
		explicit XRenderSubsystem(cContext* context);
		virtual ~XRenderSubsystem() = default;

		void Initialize();
		void Shutdown();
		void MainThreadFunction(IApplication* app);
		void NotifyMainThread();
		void PushCommand(const SRenderCommand& command);
		void Kill();

		inline CRenderFrame GetScratchFrame() const
		{
			return _scratchFrame;
		}
	};
}