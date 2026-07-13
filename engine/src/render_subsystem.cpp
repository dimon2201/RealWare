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
#include "game_object_subsystem.hpp"
#include "material_subsystem.hpp"
#include "animation_subsystem.hpp"
#include "skeleton_subsystem.hpp"
#include "skinning_subsystem.hpp"

using namespace types;

triton::XRenderSubsystem::XRenderSubsystem(cContext* context) : iObject(context)
{
	CThreadGuard::AssertMain();
}

void triton::XRenderSubsystem::Initialize()
{
	CThreadGuard::AssertMain();

	_synchronization = _context->Create<XEngineMTSynchronization>(_context, this);
	for (usize i = 0; i < 2; i++)
	{
		_synchronization->_mainThreadSwapChainSnapshot._frames[i] = EFrameState::FREE;
		_synchronization->_renderThreadSwapChainSnapshot._frames[i] = EFrameState::FREE;
	}
	
	// Create render thread
	cInputWindow& window = _context->GetSubsystem<cInput>()->GetWindows()->at(0);
	for (usize i = 0; i < 2; i++)
		_synchronization->_swapChain._frames[i].Reset(&window);
	_scratchFrame.Reset(&window);
	
	_renderThread = _context->Create<cRenderThread>(_context, _synchronization, this);
	_renderThread->Run();
	{
		std::unique_lock<std::mutex> lock(_synchronization->_mutex);
		// Wait until render thread gets initialized to continue main thread
		_cv.wait(lock, [this] { return _renderThread->IsInitialized(); });
	}
}

void triton::XRenderSubsystem::Shutdown()
{
	CThreadGuard::AssertMain();

	_context->Destroy<cRenderThread>(_renderThread);
	_context->Destroy<XEngineMTSynchronization>(_synchronization);
}

void triton::XRenderSubsystem::MainThreadFunction(IApplication* app)
{
	CThreadGuard::AssertMain();

	if (app == nullptr)
		return;

	app->Setup();

	//auto camera = _context->GetSubsystem<cCameraSystem>();
	//auto time = _context->GetSubsystem<cTime>();
	//auto physics = _context->GetSubsystem<cPhysics>();

	iInputBackend* inputBackend = _context->GetBackend<iInputBackend>();
	cInput* input = _context->GetSubsystem<cInput>();
	std::vector<cInputWindow>* windows = input->GetWindows();
	cInputWindow& window = windows->at(0);
	s32 windowCount = windows->size();
	boolean bIsRunning = K_TRUE;
	while (bIsRunning)
	{
		SEvent e = {};
		while ((e = inputBackend->PollEvent()).type != EWindowEvent::None)
		{
			if (e.type == EWindowEvent::Quit)
			{
				bIsRunning = K_FALSE;
				break;
			}
			else
			{
				inputBackend->ProcessEvent(e);
			}
		}

		_synchronization->WaitForFreeFrame(_cv);

		s32 windowCount = windows->size();
		if (windowCount == 0)
			break;

		// Prepare frame for render thread
		for (s32 i = windowCount - 1; i > -1; i--)
		{
			if (e.type != EWindowEvent::Quit)
			{
				// Fill frame
				SRenderCommand cmd = SRenderCommand(
					ERenderCommand::CLEAR,
					1.0f,
					0.0f,
					0.0f,
					0.0f,
					1.0f
				);
				PushCommand(cmd);
			}
			// Destroy window if needed
			else if (e.type == EWindowEvent::Quit)
			{
				// input->DestroyWindow(window);
				windows->erase(windows->begin() + i);

				Kill();

				_renderThread->NotifyThread();
				break;
			}
		}
		if (e.type == EWindowEvent::Quit)
			break;

		app->Update();
		_context->GetSubsystem<XGameObjectSubsystem>()->Update();
		_context->GetSubsystem<XMaterialSubsystem>()->Update();
		_context->GetSubsystem<XTextureSubsystem>()->Update();
		_context->GetSubsystem<XAnimationSubsystem>()->Update();
		_context->GetSubsystem<XSkeletonSubsystem>()->Update();
		_context->GetSubsystem<XSkinningSubsystem>()->Update();

		_synchronization->ProduceFrame(EFrameState::EXECUTE_FULL);
		ResetScratchFrame();

		_renderThread->NotifyThread();
	}

	input->DestroyWindow(&window);

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

void triton::XRenderSubsystem::PushCommand(const SRenderCommand& command)
{
	CThreadGuard::AssertMain();

	usize resultByteSize = 0;
	switch (command._command)
	{
		case ERenderCommand::CREATE_BUFFER:
			resultByteSize = sizeof(cBuffer*);
			break;
		case ERenderCommand::CREATE_TEXTURE:
			resultByteSize = sizeof(cTexture*);
			break;
		case ERenderCommand::CREATE_RENDER_TARGET:
			resultByteSize = sizeof(XRenderTarget*);
			break;
	}

	auto c = _context;
	const sCapabilities* caps = _context->GetSubsystem<cEngine>()->GetCapabilities();
	if (resultByteSize >= caps->futureResultBufferByteSize)
	{
		Print("Error: command result byte size value '" + std::to_string(resultByteSize) + "' exceeds futureResultBufferByteSize '" + std::to_string(caps->futureResultBufferByteSize) + "'!");
		return;
	}

	_scratchFrame.PushCommand(command);
}

void triton::XRenderSubsystem::Kill()
{
	CThreadGuard::AssertMain();

	_synchronization->Kill();
}