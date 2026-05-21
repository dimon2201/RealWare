// render_frame.hpp

#pragma once

#include <vector>
#include <optional>
#include <mutex>
#include "object.hpp"
#include "types.hpp"

namespace triton
{
	class cInputWindow;
	class XRenderSubsystem;

	enum class ERenderCommand
	{
		NONE = 0,
		RESIZE_RENDER_TARGETS,
		CLEAR,
	};

	enum class EFrameState
	{
		READY = 0,
		CONSUMED
	};

	enum class EFrameOperation
	{
		ACQUIRE = 0,
		STOP_EXECUTION
	};

	class SRenderCommandArgs
	{
	public:
		types::cpuword _argA = 0;
		types::cpuword _argB = 0;
		types::cpuword _argC = 0;
		types::cpuword _argD = 0;
	};

	class SRenderCommand
	{
	public:
		ERenderCommand _command = ERenderCommand::NONE;
		SRenderCommandArgs _args = {};
	};

	class alignas(64) CRenderFrame final
	{
		EFrameOperation _op = EFrameOperation::ACQUIRE;
		cInputWindow* _window = nullptr;
		std::vector<SRenderCommand> _commands;
		mutable types::usize _nextCommandIndex = 0;

	public:
		explicit CRenderFrame(cInputWindow* window) : _window(window) {}

		void Reset(cInputWindow* window = nullptr, EFrameOperation op = EFrameOperation::ACQUIRE);
		void PushCommand(const SRenderCommand& command);
		std::optional<const SRenderCommand*> Next() const;

		inline EFrameOperation GetOperation() const
		{
			return _op;
		}

		inline cInputWindow* GetWindow() const
		{
			return _window;
		}
	};

	class SFrameSwapChain
	{
	public:
		CRenderFrame _frames[2] = { CRenderFrame(nullptr),  CRenderFrame(nullptr) };
	};

	class XFrameSync final : public iObject
	{
		TRITON_OBJECT(XFrameSync)

		XRenderSubsystem* _renderSubsystem = nullptr;
		SFrameSwapChain _frameSwapChain = {};
		types::u32 _frontIndex = 0;
		types::u32 _backIndex = 0;
		EFrameState _state = EFrameState::CONSUMED;
		std::mutex _mutex;

	public:
		explicit XFrameSync(cContext* context, XRenderSubsystem* renderSubsystem) : iObject(context), _renderSubsystem(renderSubsystem) {}
		virtual ~XFrameSync() = default;

		void CopyFrame();

		types::u32 WaitUntilReady(std::condition_variable& cv);
		void WaitUntilConsumed(std::condition_variable& cv);
		std::optional<const CRenderFrame*> AcquireFrame();
		void Consume();
		void Publish();
	};
}