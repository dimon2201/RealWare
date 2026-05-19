// frame_render_susbsytem.hpp

#include <mutex>
#include <condition_variable>
#include "object.hpp"
#include "types.hpp"

namespace triton
{
	class IApplication;
	class cRenderThread;
	class cRenderThreadState;

	class XFrameRenderSubsystem final : public iObject
	{
		TRITON_OBJECT(XFrameRenderSubsystem)

		cRenderThreadState* _renderThreadStateBuffer = nullptr;
		cRenderThread* _renderThread = nullptr;
		types::u32 _frontIndex = 0;
		types::u32 _backIndex = 0;
		std::atomic<types::boolean> _frameReady;
		std::mutex _threadMutex;
		std::condition_variable _cv;

	public:
		explicit XFrameRenderSubsystem(cContext* context);
		virtual ~XFrameRenderSubsystem() = default;

		void Initialize();
		void Shutdown();
		void MainThreadFunction(IApplication* app);
		void NotifyMainThread();

		inline types::boolean IsFrameReady() const
		{
			return _frameReady.load();
		}

		inline types::u32 GetFrontIndex() const
		{
			return _frontIndex;
		}

		inline types::u32 GetBackIndex() const
		{
			return _backIndex;
		}

		inline void MarkFrameReady(types::boolean value)
		{
			_frameReady.store(value);
		}

		inline cRenderThreadState* GetRenderThreadStateBuffer() const
		{
			return _renderThreadStateBuffer;
		}
	};
}