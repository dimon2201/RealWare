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
	class cTexture;
	template <typename T>
	class cStack;

	struct SRenderCommandResult final
	{
		types::usize bufferByteOffset = 0;
		types::usize bufferByteSize = 0;
	};

	class XRenderSubsystem final : public iObject
	{
		TRITON_OBJECT(XRenderSubsystem)

	private:
		XEngineMTSynchronization* _synchronization = nullptr;
		cRenderThread* _renderThread = nullptr;
		std::condition_variable _cv;
		CRenderFrame _scratchFrame = CRenderFrame(nullptr);

		// TODO: encapsulate this in separate CResultBuffer class
		types::u8* _resultBuffer = nullptr;
		types::usize _nextResultBufferByte = 0;
		cStack<SRenderCommandResult>* _freeResults = nullptr;

		void DiscardResult(const SRenderCommandResult& result);

	public:
		explicit XRenderSubsystem(cContext* context);
		virtual ~XRenderSubsystem() = default;

		void Initialize();
		void Shutdown();
		void MainThreadFunction(IApplication* app);
		void NotifyMainThread();
		SRenderCommandResult PushCommand(const SRenderCommand& command);
		void Kill();

		template <typename TResult>
		TResult FetchResult(const SRenderCommandResult& result) const
		{
			_synchronization->WaitForResult(_cv, _renderThread);

			TResult r = {};
			memcpy(&r, &_resultBuffer[result.bufferByteOffset], sizeof(TResult));

			DiscardResult(result);

			return r;
		}

		inline CRenderFrame GetScratchFrame() const
		{
			return _scratchFrame;
		}
	};
}