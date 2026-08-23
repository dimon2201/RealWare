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
	class cContext;
	class IGraphicsBackend2;
	class CGraphics;
	class XSynchronization;
	class CWindow;
	struct SRenderCommandPack;

	class cRenderThread final : public cThread
	{
		cContext* _context = nullptr;
		XSynchronization* _sync = nullptr;
		CWindow* _window = nullptr;

	public:
		explicit cRenderThread(
			cContext* context,
			XSynchronization* sync,
			CWindow* window
		) : _context(context), _sync(sync), _window(window) {}

		~cRenderThread() override = default;

		virtual void ThreadFunction() override;

	private:
		void ExecuteCommands(
			const SRenderCommandPack& renderCommandPack,
			IGraphicsBackend2* gfxBackend
		);

		void Present(IGraphicsBackend2* gfxBackend);
	};
}
