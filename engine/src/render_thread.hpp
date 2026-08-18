// render_thread.hpp

#pragma once

#include <cstring>
#include <atomic>
#include <cassert>
#include <thread>
#include "thread_subsystem.hpp"
#include "synchronization.hpp"
#include "types.hpp"

namespace triton
{
	class IGraphicsBackend;
	class XGraphics;
	class XSynchronization;
	struct SRenderCommandPack;

	class cRenderThread final : public cThread
	{
		TRITON_OBJECT(cRenderThread)

		XSynchronization* _sync = nullptr;

	public:
		explicit cRenderThread(
			cContext* context,
			XSynchronization* sync
		);
		~cRenderThread() override;

		virtual void ThreadFunction() override;

	private:
		void ExecuteCommands(
			const SRenderCommandPack& renderCommandPack,
			IGraphicsBackend* gfxBackend,
			XGraphics* gfx
		);

		void Present(const cInputWindow* window, IGraphicsBackend* gfxBackend);
	};
}
