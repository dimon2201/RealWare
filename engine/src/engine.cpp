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
#include "thread_manager.hpp"
#include "render_context.hpp"
#include "audio.hpp"
#include "math.hpp"
#include "ecs.hpp"
#include "input_glfw_backend.hpp"

using namespace triton::ecs;
using namespace types;

triton::cEngine::cEngine(cContext* context, iApplication* app, cStack<cInputWindow>*& appWindows) : iObject(context), _app(app)
{
	if (_app != nullptr)
		_caps = _app->GetCapabilities();

	Initialize(appWindows);
}

void triton::cEngine::Initialize(cStack<cInputWindow>*& appWindows)
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

	// Register backends
	_context->RegisterBackend<iInputBackend>(new cInputGLFWBackend(_context));
	_context->RegisterBackend<iGraphicsBackend>(new cOGLGraphicsBackend(_context));

	// Register subsystems
	_context->RegisterSubsystem(this);
	_context->RegisterSubsystem(new cInput(_context));
	_context->RegisterSubsystem(new cGraphics(_context));
	_context->RegisterSubsystem(new cTextureAtlas(_context));
	_context->RegisterSubsystem(new cFileSystem(_context));
	//_context->RegisterSubsystem(new cFont(_context));
	//_context->RegisterSubsystem(new cPhysics(_context));
	//_context->RegisterSubsystem(new cThread(_context));
	//_context->RegisterSubsystem(new cTime(_context));
	//_context->RegisterSubsystem(new cEventDispatcher(_context));
	//_context->RegisterSubsystem(new cMath(_context));
	//_context->RegisterSubsystem(new cECSSystem(_context));

	// Create windows
	if (appWindows != nullptr)
	{
		Print("Error: application windows already created!");

		return;
	}

	sChunkAllocatorDescriptor cad = {};
	cad.chunkByteSize = _caps->hashTableChunkByteSize;
	cad.maxChunkCount = _caps->hashTableMaxChunkCount;
	cad.hashTableSize = _caps->hashTableSize;
	appWindows = new cStack<cInputWindow>(_context, cad);

	cInput* input = _context->GetSubsystem<cInput>();
	if (input == nullptr)
		return;

	for (usize i = 0; i < _caps->windowCount; i++)
	{
		cInputWindow* window = input->CreatePlatformWindow(
			_caps->windows[i].windowTitle,
			cVector2(_caps->windows[i].windowWidth, _caps->windows[i].windowHeight),
			_caps->windows[i].fullscreen
		);

		appWindows->Push(*window);
	}

	// Initialize subsystems
	// NOTE: order matters
	_context->GetSubsystem<cTextureAtlas>()->Initialize(cVector3(2048, 2048, 16));
	_context->GetSubsystem<cGraphics>()->Initialize();

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
}

void triton::cEngine::Run()
{
	if (_app == nullptr)
		return;

	_app->Setup();

	//auto gfx = _context->GetSubsystem<cGraphics>();
	//auto input = _context->GetSubsystem<cInput>();
	//auto camera = _context->GetSubsystem<cCameraSystem>();
	//auto time = _context->GetSubsystem<cTime>();
	//auto physics = _context->GetSubsystem<cPhysics>();

	//cInputWindow* window = _app->GetWindow();

	//time->BeginFrame();

	//while (window->GetRunState() == K_FALSE)
	//{
	//	time->Update();
		// physics->Simulate(); TODO: physics simulation
		// camera->OnFrameUpdate(); TODO: camera system per frame update
	//	gfx->CompositeFinal();
	//	window->SwapBuffers();
	//	window->PollEvents();
	//}

	//time->EndFrame();

	iInputBackend* input = _context->GetBackend<iInputBackend>();

	cStack<cInputWindow>* windows = GetApplication()->GetWindows();
	usize windowCount = windows->GetSize();
	for (usize closedWindowCounter = 0; closedWindowCounter < windowCount; closedWindowCounter = 0)
	{
		for (usize i = 0; i < windowCount; i++)
		{
			cInputWindow* window = windows->At(i);
			if (windows->At(i)->GetRunState() == cInputWindow::eRunState::OPENED)
				window->SwapBuffers();
			else
				closedWindowCounter++;
		}

		input->PollEvents();
	}

	_app->Stop();
}