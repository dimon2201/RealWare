// render_susbsytem.hpp

namespace triton
{
	class cRenderThread;
	class cRenderThreadState;

	class CFrameRenderSubsystem final
	{
		TRITON_OBJECT(CFrameRenderSubsystem)
		
		cRenderThreadState* _renderThreadStateBuffer = nullptr;
		types::u32 _frontIndex = 0;
		types::u32 _backIndex = 0;
		std::atomic<types::boolean> _frameReady;
		cRenderThread* _renderThread = nullptr;
		std::mutex _threadMutex;
		std::condition_variable _cv;

	public:
		explicit CFrameRenderSubsystem(cContext* context);
		virtual ~CFrameRenderSubsystem();

		void Run(IApplication* app);
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