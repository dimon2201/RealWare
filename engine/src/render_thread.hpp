// render_thread.hpp

#pragma once

#include <atomic>
#include "thread_subsystem.hpp"

namespace triton
{
	class cRenderThread final : public cThread
	{
		cEngine* _engine = nullptr;
		std::atomic<types::boolean> _initialized = types::K_FALSE;

	public:
		explicit cRenderThread(cContext* context, cEngine* engine);
		virtual ~cRenderThread() = default;

		virtual void ThreadFunction() override;

		inline types::boolean IsInitialized() const { return _initialized.load(); }
	};
}