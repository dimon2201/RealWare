// engine.cpp

#include "engine.hpp"
#include "application.hpp"
#include "context.hpp"
#include "graphics.hpp"
#include "input.hpp"
#include "camera_system.hpp"
#include "texture_manager.hpp"
#include "filesystem_manager.hpp"
#include "font_manager.hpp"
#include "physics_manager.hpp"
#include "time.hpp"
#include "event_manager.hpp"
#include "gameobject_manager.hpp"
#include "thread_subsystem.hpp"
#include "audio.hpp"
#include "math.hpp"
#include "ecs.hpp"
#include "input_backend_glfw.hpp"
#include "graphics_resource_backend_ogl.hpp"
#include "graphics_pipeline_backend_ogl.hpp"
#include "graphics_context_backend_ogl.hpp"
#include "graphics_drawcall_backend_ogl.hpp"
#include "audio_backend_oal.hpp"
#include "render_thread.hpp"

using namespace triton::ecs;
using namespace types;

triton::CEngineMultithreadedExecution::CEngineMultithreadedExecution(cContext* context) : iObject(context)
{
	_frontIndex = 0; // render thread reads it
	_backIndex = 1; // main thread writes it
	_frameReady.store(K_FALSE);

	// Create render thread
	_renderThreadStateBuffer = (cRenderThreadState*)_context->GetMemoryAllocator()->Allocate(sizeof(cRenderThreadState) * 2, 64);
	{
		std::unique_lock<std::mutex> lock(_threadMutex);
		_renderThread = _context->Create<cRenderThread>(_context, this);
		_renderThread->Run();

		// Wait until render thread gets initialized to continue main thread
		_cv.wait(lock, [this] { return _renderThread->IsInitialized(); });
	}
}

triton::CEngineMultithreadedExecution::~CEngineMultithreadedExecution()
{
	_context->GetMemoryAllocator()->Deallocate(_renderThreadStateBuffer);
	_context->Destroy<cRenderThread>(_renderThread);
}

void triton::CEngineMultithreadedExecution::Run(iApplication* app)
{
	if (app == nullptr)
		return;

	app->Setup();

	auto gfx = _context->GetSubsystem<cGraphics>();
	//auto camera = _context->GetSubsystem<cCameraSystem>();
	auto time = _context->GetSubsystem<cTime>();
	//auto physics = _context->GetSubsystem<cPhysics>();

	time->BeginFrame();

	iInputBackend* inputBackend = _context->GetBackend<iInputBackend>();
	cInput* input = _context->GetSubsystem<cInput>();
	cStack<cInputWindow>* windows = input->GetWindows();
	s32 windowCount = windows->GetSize();
	while (K_TRUE)
	{
		inputBackend->PollEvents();

		s32 windowCount = windows->GetSize();
		if (windowCount == 0)
			break;

		cRenderThreadState& renderThreadState = _renderThreadStateBuffer[_backIndex];
		renderThreadState.Reset();

		// Prepare frame for render thread
		for (s32 i = windowCount - 1; i > -1; i--)
		{
			cInputWindow* window = windows->At(i);
			if (window->GetRunState() == cInputWindow::eRunState::OPENED)
			{
				// Fill render commands for render thread
				renderThreadState.PushWindow(window);
				sRenderCommandArgs args;
				renderThreadState.PushCommand(eRenderCommand::CLEAR_SCREEN, std::move(args));
			}
			// Destroy window if needed
			else if (window->GetRunState() == cInputWindow::eRunState::CLOSED)
			{
				input->DestroyWindow(window);
				windows->Erase(i);
			}
		}

		// Publish frame
		{
			std::lock_guard<std::mutex> lock(_threadMutex);
			std::swap(_frontIndex, _backIndex);
			_frameReady.store(K_TRUE);
		}
		_renderThread->NotifyThread();
	}

	time->EndFrame();

	// Stop render thread
	_renderThread->Stop();

	app->Stop();
}

void triton::CEngineMultithreadedExecution::NotifyMainThread()
{
	_cv.notify_one();
}

triton::cEngine::cEngine(cContext* context, iApplication* app) : iObject(context), _app(app)
{
	if (_app != nullptr)
		_caps = _app->GetCapabilities();

	Initialize();
}

triton::cEngine::~cEngine()
{
	Shutdown();
}

void triton::cEngine::Initialize()
{
	// Create memory allocator
	_context->CreateMemoryAllocator();

	// Register factories
	_context->RegisterFactory<cInputWindow>();
	_context->RegisterFactory<cStack<cInputWindow>>();
	_context->RegisterFactory<cBuffer>();
	_context->RegisterFactory<cTexture>();
	_context->RegisterFactory<cRenderTarget>();
	_context->RegisterFactory<cShader>();
	_context->RegisterFactory<cVertexArray>();
	_context->RegisterFactory<cRenderTarget>();
	_context->RegisterFactory<cRenderPass>();
	_context->RegisterFactory<cRenderPassGPU>();
	_context->RegisterFactory<cDataBuffer>();
	_context->RegisterFactory<cDataFile>();
	_context->RegisterFactory<CEngineMultithreadedExecution>();
	_context->RegisterFactory<cRenderThread>();

	// Register backends
	_context->RegisterBackend<iInputBackend>(new cInputBackendGLFW(_context));
	_context->RegisterBackend<iGraphicsResourceBackend>(new cGraphicsResourceBackendOGL(_context));
	_context->RegisterBackend<iGraphicsPipelineBackend>(new cGraphicsPipelineBackendOGL(_context));
	_context->RegisterBackend<iGraphicsContextBackend>(new cGraphicsContextBackendOGL(_context));
	_context->RegisterBackend<iGraphicsDrawcallBackend>(new cGraphicsDrawcallBackendOGL(_context));
	_context->RegisterBackend<iAudioBackend>(new cAudioBackendOAL(_context));

	// Register subsystems
	_context->RegisterSubsystem(this);
	_context->RegisterSubsystem(new cInput(_context));
	_context->RegisterSubsystem(new cGraphics(_context));
	_context->RegisterSubsystem(new cAudio(_context));
	_context->RegisterSubsystem(new cTextureAtlas(_context));
	_context->RegisterSubsystem(new cFileSystem(_context));
	_context->RegisterSubsystem(new cTime(_context));
	_context->RegisterSubsystem(new cThreadSubsystem(_context));
	//_context->RegisterSubsystem(new cFont(_context));
	//_context->RegisterSubsystem(new cPhysics(_context));
	//_context->RegisterSubsystem(new cThread(_context));
	//_context->RegisterSubsystem(new cTime(_context));
	//_context->RegisterSubsystem(new cEventDispatcher(_context));
	//_context->RegisterSubsystem(new cMath(_context));
	//_context->RegisterSubsystem(new cECSSystem(_context));

	// Initialize subsystems
	_context->GetSubsystem<cInput>()->Initialize();

	// Create systems
	//cAudio* audioSystem = _context->Create<cAudio>(_context, cAudio::API::OAL);
	//cCameraSystem* camera = _context->Create<cCameraSystem>(_context);

	// Subscribe systems to core events
	//audioSystem->Subscribe(
	//	eEventType::FRAME_UPDATE,
	//	[audioSystem] (iObject* self, cContext* context, cDataBuffer* data) {
	//		audioSystem->OnFrameUpdate();
	//	}
	//);

	// Create texture manager
	//cTextureAtlas* texture = _context->GetSubsystem<cTextureAtlas>();
	//texture->SetAtlas(glm::vec3(2048, 2048, 16));

	// Create sound context
	//cAudio* audio = _context->GetSubsystem<cAudio>();

	_execution = _context->Create<CEngineMultithreadedExecution>(_context);
}

void triton::cEngine::Shutdown()
{
	_context->Destroy<CEngineMultithreadedExecution>(_execution);

	_context->GetSubsystem<cGraphics>()->Shutdown();
	_context->GetSubsystem<cTextureAtlas>()->Shutdown();
	_context->GetSubsystem<cInput>()->Shutdown();
}

void triton::cEngine::Run()
{
	_execution->Run(_app);
}