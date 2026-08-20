// render_frame.cpp

#include "synchronization.hpp"
#include "thread_guard.hpp"
#include "render_thread.hpp"
#include "context.hpp"
#include "input.hpp"
#include "object_allocator.hpp"

using namespace types;

triton::XSynchronization::XSynchronization(
	cContext* context,
	XRenderCommandRecorder* cmdRecorder,
	usize resultBufferByteSize
) : _context(context), _cmdRecorder(cmdRecorder)
{
	_resultBuffer.data = (u8*)CObjectAllocator::Allocate(resultBufferByteSize, 64);
}

triton::XSynchronization::~XSynchronization()
{
	CObjectAllocator::Deallocate(_resultBuffer.data);
}

void triton::XSynchronization::WaitForRenderThreadToInit()
{
	CThreadGuard::AssertMain();

	_bIsRenderThreadInitialized.wait(K_FALSE, std::memory_order_acquire);
}

void triton::XSynchronization::WaitForReleasedFrame()
{
	CThreadGuard::AssertMain();

	_frameAtomics.releasedFrame.wait(
		_frameAtomics.lastReleasedFrame,
		std::memory_order_acquire
	);
	_frameAtomics.lastReleasedFrame = _frameAtomics.releasedFrame.load(std::memory_order_acquire);
}

void triton::XSynchronization::WaitForProducedFrame()
{
	CThreadGuard::AssertRender();

	_frameAtomics.producedFrame.wait(
		_frameAtomics.lastProducedFrame,
		std::memory_order_acquire
	);
	_frameAtomics.lastProducedFrame = _frameAtomics.producedFrame.load(std::memory_order_acquire);
}

void triton::XSynchronization::WaitForRenderJobFinish()
{
	CThreadGuard::AssertMain();

	_bIsRenderJobFinished.wait(K_FALSE, std::memory_order_acquire);
	_bIsRenderJobFinished.store(K_FALSE, std::memory_order_release);
}

void triton::XSynchronization::InitRenderThread()
{
	CThreadGuard::AssertRender();

	_bIsRenderThreadInitialized.store(K_TRUE, std::memory_order_release);
	_bIsRenderThreadInitialized.notify_one();
}

void triton::XSynchronization::ProduceFrame(
	EProducedFrameOp operation,
	const SRenderCommandPack& renderCommandPack,
	cInputWindow* window
)
{
	CThreadGuard::AssertMain();

	const usize writeIndex = _writeIndex;
	_producedFrameData[writeIndex].Clear();
	_producedFrameData[writeIndex].Apply(operation, renderCommandPack, window);
	_readIndex.store(writeIndex, std::memory_order_release);
	_writeIndex = writeIndex ^ 1;

	_frameAtomics.producedFrame.fetch_add(1, std::memory_order_release);
	_frameAtomics.producedFrame.notify_one();
}

void triton::XSynchronization::ReleaseFrame()
{
	CThreadGuard::AssertRender();

	_frameAtomics.releasedFrame.fetch_add(1, std::memory_order_release);
	_frameAtomics.releasedFrame.notify_one();

	_cmdRecorder->Clear();
}