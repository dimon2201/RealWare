// render_thread.hpp

#pragma once

#include "thread_subsystem.hpp"

namespace triton
{
	class cRenderThread final : public cThread
	{
	public:
		explicit cRenderThread(cContext* context);
		virtual ~cRenderThread() = default;

		virtual void ThreadFunction() override;
	};
}