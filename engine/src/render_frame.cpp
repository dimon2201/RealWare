// render_frame.cpp

#include "render_frame.hpp"
#include "render_subsystem.hpp"
#include "thread_guard.hpp"

void triton::CRenderFrame::Reset(cInputWindow* window, EFrameOperation op)
{
	if (op != EFrameOperation::ACQUIRE)
		_op = op;
	if (window != nullptr)
		_window = window;
	_commands.shrink_to_fit();
	_nextCommandIndex = 0;
}

void triton::CRenderFrame::PushCommand(const SRenderCommand& command)
{
	_commands.push_back(command);
}

std::optional<const triton::SRenderCommand*> triton::CRenderFrame::Next() const
{
	if (_nextCommandIndex >= _commands.size())
		return std::nullopt;

	return &_commands.data()[_nextCommandIndex++];
}

void triton::XFrameSync::CopyFrame()
{
	CThreadGuard::AssertMain();

	_frameSwapChain._frames[_backIndex].Reset();
	_frameSwapChain._frames[_backIndex] = _renderSubsystem->GetExternalFrame();
}

types::u32 triton::XFrameSync::WaitUntilReady(std::condition_variable& cv)
{
	std::unique_lock<std::mutex> lock(_mutex);
	cv.wait(lock, [this] {
		return _state == EFrameState::READY;
	});

	return _frontIndex;
}

void triton::XFrameSync::WaitUntilConsumed(std::condition_variable& cv)
{
	std::unique_lock<std::mutex> lock(_mutex);
	cv.wait(lock, [this] { return _state == EFrameState::CONSUMED; });
}

std::optional<const triton::CRenderFrame*> triton::XFrameSync::AcquireFrame()
{
	const CRenderFrame* renderFrame = &_frameSwapChain._frames[_frontIndex];
	if (renderFrame->GetOperation() == EFrameOperation::STOP_EXECUTION)
		return std::nullopt;

	return renderFrame;
}

void triton::XFrameSync::Consume()
{
	std::lock_guard<std::mutex> lock(_mutex);
	_state = EFrameState::CONSUMED;
}

void triton::XFrameSync::Publish()
{
	std::lock_guard<std::mutex> lock(_mutex);
	std::swap(_frontIndex, _backIndex);
	_state = EFrameState::READY;
}