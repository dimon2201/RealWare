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

triton::cEngine::cEngine(cContext* context, iApplication* app) : iObject(context), _app(app)
{
	if (_app != nullptr)
		_caps = _app->GetCapabilities();

	Initialize();
}

void triton::cEngine::Initialize()
{
	// Create memory allocator
	_context->CreateMemoryAllocator();

	// Register factories
	_context->RegisterFactory<cInputWindow>();
	_context->RegisterFactory<cStack<cInputWindow>>();
	//_context->RegisterFactory<cBuffer>();
	//_context->RegisterFactory<cShader>();
	//_context->RegisterFactory<cTexture>();
	//_context->RegisterFactory<cRenderTarget>();
	//_context->RegisterFactory<cRenderPass>();

	// Register backends
	_context->RegisterBackend<iInputBackend>(new cInputGLFWBackend(_context));
	_context->RegisterBackend<iGraphicsBackend>(new cOGLGraphicsBackend(_context));

	// Register subsystems
	_context->RegisterSubsystem(this);
	_context->RegisterSubsystem(new cInput(_context));
	//_context->RegisterSubsystem(new cGraphics(_context, cGraphics::eAPI::OGL));
	//_context->RegisterSubsystem(new cTextureAtlas(_context));
	//_context->RegisterSubsystem(new cFileSystem(_context));
	//_context->RegisterSubsystem(new cFont(_context));
	//_context->RegisterSubsystem(new cPhysics(_context));
	//_context->RegisterSubsystem(new cThread(_context));
	//_context->RegisterSubsystem(new cTime(_context));
	//_context->RegisterSubsystem(new cEventDispatcher(_context));
	//_context->RegisterSubsystem(new cMath(_context));
	//_context->RegisterSubsystem(new cECSSystem(_context));

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