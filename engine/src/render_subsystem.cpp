// render_susbsytem.cpp

#include "render_subsystem.hpp"
#include "context.hpp"
#include "graphics.hpp"
#include "time.hpp"
#include "render_thread.hpp"
#include "application.hpp"
#include "input_backend.hpp"
#include "input.hpp"
#include "thread_guard.hpp"
#include "stack.hpp"
#include "capabilities.hpp"
#include "log.hpp"

using namespace types;

triton::XRenderSubsystem::XRenderSubsystem(cContext* context) : iObject(context)
{
	CThreadGuard::AssertMain();
}

void triton::XRenderSubsystem::Initialize()
{
	CThreadGuard::AssertMain();

	const sCapabilities* caps = _context->GetSubsystem<cEngine>()->GetCapabilities();
	_resultBuffer = (u8*)_context->GetMemoryAllocator()->Allocate(caps->futureResultBufferByteSize, 64);
	sChunkAllocatorDescriptor cad = {};
	cad.chunkByteSize = caps->hashTableChunkByteSize;
	cad.maxChunkCount = caps->hashTableMaxChunkCount;
	cad.hashTableSize = caps->hashTableSize;
	_freeResults = _context->Create<cStack<SRenderCommandResult>>(_context, cad);

	_synchronization = _context->Create<XEngineMTSynchronization>(_context, this);
	for (usize i = 0; i < 2; i++)
	{
		_synchronization->_mainThreadSwapChainSnapshot._frames[i] = EFrameState::READY;
		_synchronization->_renderThreadSwapChainSnapshot._frames[i] = EFrameState::READY;
	}
	
	// Create render thread
	cInputWindow* window = _context->GetSubsystem<cInput>()->GetWindows()->At(0).data;
	for (usize i = 0; i < 2; i++)
		_synchronization->_swapChain._frames[i].Reset(window);

	_scratchFrame.Reset(window);
	
	{
		std::unique_lock<std::mutex> lock(_synchronization->_mutex);
		_renderThread = _context->Create<cRenderThread>(_context, _synchronization, this);
		_renderThread->Run();

		// Wait until render thread gets initialized to continue main thread
		_cv.wait(lock, [this] { return _renderThread->IsInitialized(); });
	}
}

void triton::XRenderSubsystem::Shutdown()
{
	CThreadGuard::AssertMain();

	_context->Destroy<cRenderThread>(_renderThread);
	_context->Destroy<XEngineMTSynchronization>(_synchronization);
	_context->Destroy<cStack<SRenderCommandResult>>(_freeResults);
	_context->GetMemoryAllocator()->Deallocate(_resultBuffer);
}

void triton::XRenderSubsystem::MainThreadFunction(IApplication* app)
{
	CThreadGuard::AssertMain();

	if (app == nullptr)
		return;

	app->Setup();

	//auto camera = _context->GetSubsystem<cCameraSystem>();
	auto time = _context->GetSubsystem<cTime>();
	//auto physics = _context->GetSubsystem<cPhysics>();

	time->BeginFrame();

	iInputBackend* inputBackend = _context->GetBackend<iInputBackend>();
	cInput* input = _context->GetSubsystem<cInput>();
	cStack<cInputWindow>* windows = input->GetWindows();
	cInputWindow* window = windows->At(0).data;
	s32 windowCount = windows->GetSize();
	while (K_TRUE)
	{
		inputBackend->PollEvents();

		_synchronization->WaitForFreeFrame(_cv);

		s32 windowCount = windows->GetSize();
		if (windowCount == 0)
			break;

		// Prepare frame for render thread
		for (s32 i = windowCount - 1; i > -1; i--)
		{
			if (window->GetRunState() == cInputWindow::eRunState::OPENED)
			{
				// Fill frame
				SRenderCommand cmd = SRenderCommand(
					ERenderCommand::CLEAR,
					1.0f,
					0.0f,
					0.0f,
					0.0f
				);
				PushCommand(cmd);
			}
			// Destroy window if needed
			else if (window->GetRunState() == cInputWindow::eRunState::CLOSED)
			{
				// input->DestroyWindow(window);
				windows->Erase(i);

				Kill();

				_renderThread->NotifyThread();
				break;
			}
		}

		_synchronization->ProduceFrame();

		_renderThread->NotifyThread();
	}

	input->DestroyWindow(window);

	time->EndFrame();

	// Stop render thread
	// TODO: main thread must wait until render thread finishes job completely
	_renderThread->Stop();

	app->Stop();
}

void triton::XRenderSubsystem::NotifyMainThread()
{
	CThreadGuard::AssertRender();

	_cv.notify_one();
}

triton::SRenderCommandResult triton::XRenderSubsystem::PushCommand(const SRenderCommand& command)
{
	CThreadGuard::AssertMain();

	usize bufferByteSize = 0;
	switch (command._command)
	{
	case ERenderCommand::CREATE_TEXTURE:
		bufferByteSize = sizeof(cTexture*);
	}

	usize bufferByteOffset = _nextResultBufferByte;
	if (_freeResults->IsEmpty() == K_FALSE)
	{
		for (usize i = 0; i < _freeResults->GetSize(); i++)
		{
			SRenderCommandResult result = *_freeResults->At(i).data;
			if (bufferByteSize <= result.bufferByteSize)
			{
				bufferByteOffset = result.bufferByteOffset;
				break;
			}
		}
	}
	if (bufferByteOffset == _nextResultBufferByte)
		_nextResultBufferByte += bufferByteSize;
	const sCapabilities* caps = _context->GetSubsystem<cEngine>()->GetCapabilities();
	if (_nextResultBufferByte >= caps->futureResultBufferByteSize)
	{
		Print("Error: maximum futureResultBufferByteSize " + std::to_string(caps->futureResultBufferByteSize) + " exceeded!");
		return;
	}

	_scratchFrame.PushCommand(command);

	SRenderCommandResult result = {};
	result.bufferByteOffset = bufferByteOffset;
	result.bufferByteSize = bufferByteSize;

	return result;
}

void triton::XRenderSubsystem::DiscardResult(const SRenderCommandResult& result)
{
	_freeResults->Push(result);
}

void triton::XRenderSubsystem::Kill()
{
	CThreadGuard::AssertMain();

	_synchronization->Kill();
}