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
	class iGraphicsContextBackend;
	class iGraphicsDrawcallBackend;
	class iGraphicsResourceBackend;
	class iGraphicsPipelineBackend;
	class cGraphics;
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
			iGraphicsDrawcallBackend* drawcallBackend,
			iGraphicsResourceBackend* resourceBackend,
			iGraphicsPipelineBackend* pipelineBackend,
			cGraphics* gfx
		);

		void Present(const cInputWindow* window, iGraphicsContextBackend* contextBackend);
	};
}
