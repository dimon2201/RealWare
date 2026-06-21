// render_frame.cpp

#include "render_frame.hpp"
#include "render_subsystem.hpp"
#include "thread_guard.hpp"

using namespace types;

void triton::CRenderFrame::Reset(cInputWindow* window)
{
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

void triton::CRenderFrame::CopyScratchFrame(types::u32 indexInSwapChain, CRenderFrame& scratchFrame)
{
	_indexInSwapChain = indexInSwapChain;
	_window = scratchFrame._window;
	_commands = scratchFrame._commands;
	_nextCommandIndex = scratchFrame._nextCommandIndex;
}

void triton::XEngineMTSynchronization::ProduceFrame()
{
	CThreadGuard::AssertMain();

	// Find free frame
	u32 writeIndex = 0;
	for (usize i = 0; i < 2; i++)
	{
		if (_mainThreadSwapChainSnapshot._frames[i] == EFrameState::READY)
		{
			writeIndex = i;
			break;
		}
	}
	_mainThreadSwapChainSnapshot._frames[writeIndex] = EFrameState::BUSY;
	{
		std::lock_guard<std::mutex> lock(_mutex);
		_renderThreadSwapChainSnapshot._frames[writeIndex] = EFrameState::BUSY;
	}

	// Publish frame
	_swapChain._frames[writeIndex].Reset();
	_swapChain._frames[writeIndex].CopyScratchFrame(writeIndex, _renderSubsystem->GetScratchFrame());
}

void triton::XEngineMTSynchronization::ReleaseFrame(types::u32 frameIndex)
{
	CThreadGuard::AssertRender();

	_renderThreadSwapChainSnapshot._frames[frameIndex] = EFrameState::READY;
	{
		std::lock_guard<std::mutex> lock(_mutex);
		_mainThreadSwapChainSnapshot._frames[frameIndex] = EFrameState::READY;
	}
}

void triton::XEngineMTSynchronization::WaitForFreeFrame(std::condition_variable& cv)
{
	CThreadGuard::AssertMain();

	std::unique_lock<std::mutex> lock(_mutex);
	cv.wait(lock, [this] {
		return _mainThreadSwapChainSnapshot._frames[0] == EFrameState::READY ||
			_mainThreadSwapChainSnapshot._frames[1] == EFrameState::READY;
	});
}

void triton::XEngineMTSynchronization::WaitForProducedFrame(std::condition_variable& cv)
{
	CThreadGuard::AssertRender();

	std::unique_lock<std::mutex> lock(_mutex);
	cv.wait(lock, [this] {
		return _renderThreadSwapChainSnapshot._stopSync == K_TRUE ||
			_renderThreadSwapChainSnapshot._frames[0] == EFrameState::BUSY ||
			_renderThreadSwapChainSnapshot._frames[1] == EFrameState::BUSY;
	});
}

types::boolean triton::XEngineMTSynchronization::IsAlive()
{
	CThreadGuard::AssertRender();

	return _renderThreadSwapChainSnapshot._stopSync == K_FALSE ? K_TRUE : K_FALSE;
}

const triton::CRenderFrame* triton::XEngineMTSynchronization::AcquireProducedFrame()
{
	CThreadGuard::AssertRender();

	u32 readIndex = 0;
	for (usize i = 0; i < 2; i++)
	{
		if (_renderThreadSwapChainSnapshot._frames[i] == EFrameState::BUSY)
		{
			readIndex = i;
			break;
		}
	}

	const CRenderFrame* renderFrame = &_swapChain._frames[readIndex];

	return renderFrame;
}

void triton::XEngineMTSynchronization::Kill()
{
	CThreadGuard::AssertMain();

	_mainThreadSwapChainSnapshot._stopSync = K_TRUE;
	{
		std::lock_guard<std::mutex> lock(_mutex);
		_renderThreadSwapChainSnapshot._stopSync = K_TRUE;
	}
}