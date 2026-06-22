// render_frame.hpp

#pragma once

#include <vector>
#include <optional>
#include <mutex>
#include <tuple>
#include "object.hpp"
#include "types.hpp"

namespace triton
{
	class cInputWindow;
	class XRenderSubsystem;
	class cRenderThread;

	enum class ERenderCommand
	{
		NONE = 0,
		RESIZE_RENDER_TARGETS,
		CLEAR,
		DRAW,
		CREATE_BUFFER,
		WRITE_BUFFER,
		DESTROY_BUFFER,
		CREATE_TEXTURE,
		CREATE_RENDER_TARGET
	};

	enum class EFrameState
	{
		READY = 0,
		BUSY
	};

	struct SRenderCommandArgs
	{
		SRenderCommandArgs(
			types::cpuword argA_ = 0,
			types::cpuword argB_ = 0,
			types::cpuword argC_ = 0,
			types::cpuword argD_ = 0,
			types::cpuword argE_ = 0,
			types::cpuword argF_ = 0,
			types::cpuword argG_ = 0
		) : _argA(argA_), _argB(argB_), _argC(argC_), _argD(argD_), _argE(argE_), _argF(argF_), _argG(argG_) {}

		types::cpuword _argA = 0;
		types::cpuword _argB = 0;
		types::cpuword _argC = 0;
		types::cpuword _argD = 0;
		types::cpuword _argE = 0;
		types::cpuword _argF = 0;
		types::cpuword _argG = 0;
	};

	struct SRenderCommand
	{
		SRenderCommand(
			ERenderCommand command_,
			types::cpuword argA_ = 0,
			types::cpuword argB_ = 0,
			types::cpuword argC_ = 0,
			types::cpuword argD_ = 0,
			types::cpuword argE_ = 0,
			types::cpuword argF_ = 0,
			types::cpuword argG_ = 0
		) : _command(command_), _args(argA_, argB_, argC_, argD_, argE_, argF_, argG_) {}

		ERenderCommand _command = ERenderCommand::NONE;
		SRenderCommandArgs _args = {};
	};

	class alignas(64) CRenderFrame final
	{
		types::u32 _indexInSwapChain = 0;
		cInputWindow* _window = nullptr;
		std::vector<SRenderCommand> _commands;
		mutable types::usize _nextCommandIndex = 0;

	public:
		explicit CRenderFrame(cInputWindow* window) : _window(window) {}
		CRenderFrame& operator=(const CRenderFrame& rhs) = delete;

		void Reset(cInputWindow* window = nullptr);
		void PushCommand(const SRenderCommand& command);
		std::optional<const SRenderCommand*> Next() const;
		void CopyScratchFrame(types::u32 indexInSwapChain, CRenderFrame& scratchFrame);

		inline types::u32 GetIndexInSwapChain() const
		{
			return _indexInSwapChain;
		}

		inline cInputWindow* GetWindow() const
		{
			return _window;
		}
	};

	class SFrameDoubleBuffer
	{
	public:
		CRenderFrame _frames[2] = { CRenderFrame(nullptr), CRenderFrame(nullptr) };
	};

	class SFrameDoubleBufferSnapshot
	{
	public:
		types::boolean _stopSync = types::K_FALSE;
		EFrameState _frames[2] = { EFrameState::READY, EFrameState::READY };
	};

	class XEngineMTSynchronization final : public iObject
	{
		TRITON_OBJECT(XEngineMTSynchronization)

		XRenderSubsystem* _renderSubsystem = nullptr;
		SFrameDoubleBuffer _swapChain = {};
		SFrameDoubleBufferSnapshot _mainThreadSwapChainSnapshot = {}; // TODO: this one needs only _stopSync && _frameState (READY, BUSY)
		SFrameDoubleBufferSnapshot _renderThreadSwapChainSnapshot = {}; // TODO: this one needs only _stopSync && _frameState (READY, BUSY)
		std::mutex _mutex;

	public:
		explicit XEngineMTSynchronization(cContext* context, XRenderSubsystem* renderSubsystem) : iObject(context), _renderSubsystem(renderSubsystem) {}
		virtual ~XEngineMTSynchronization() override = default;

		void ProduceFrame();
		void ReleaseFrame(types::u32 frameIndex);
		void WaitForFreeFrame(std::condition_variable& cv);
		void WaitForProducedFrame(std::condition_variable& cv);
		void WaitForResult(std::condition_variable& cv, cRenderThread* renderThread);
		types::boolean IsAlive();
		const triton::CRenderFrame* AcquireProducedFrame();
		void Kill();
	};
}