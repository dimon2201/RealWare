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
#include "render_subsystem.hpp"
#include "thread_guard.hpp"

using namespace triton::ecs;
using namespace types;

triton::cEngine::cEngine(cContext* context, IApplication* app) : iObject(context), _app(app)
{
	if (_app != nullptr)
		_caps = _app->GetCapabilities();
}

triton::cEngine::~cEngine()
{
	Shutdown();
}

void triton::cEngine::Initialize()
{
	CThreadGuard::CaptureMainThreadId();

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
	_context->RegisterFactory<XRenderPass>();
	_context->RegisterFactory<XRenderPassGPU>();
	_context->RegisterFactory<XDataBuffer>();
	_context->RegisterFactory<cDataFile>();
	_context->RegisterFactory<cRenderThread>();
	_context->RegisterFactory<XFrameSync>();

	// Register backends
	_context->RegisterBackend<iInputBackend>(new cInputBackendGLFW(_context));
	_context->RegisterBackend<iGraphicsResourceBackend>(new cGraphicsResourceBackendOGL(_context));
	_context->RegisterBackend<iGraphicsPipelineBackend>(new cGraphicsPipelineBackendOGL(_context));
	_context->RegisterBackend<iGraphicsContextBackend>(new cGraphicsContextBackendOGL(_context));
	_context->RegisterBackend<iGraphicsDrawcallBackend>(new cGraphicsDrawcallBackendOGL(_context));
	_context->RegisterBackend<iAudioBackend>(new cAudioBackendOAL(_context));

	// Register subsystems
	_context->RegisterSubsystem(this);
	_context->RegisterSubsystem(new XRenderSubsystem(_context));
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
	// NOTE: order matters
	_context->GetSubsystem<cInput>()->Initialize();
	_context->GetSubsystem<XRenderSubsystem>()->Initialize();

	// Create systems
	//cAudio* audioSystem = _context->Create<cAudio>(_context, cAudio::API::OAL);
	//cCameraSystem* camera = _context->Create<cCameraSystem>(_context);

	// Subscribe systems to core events
	//audioSystem->Subscribe(
	//	eEventType::FRAME_UPDATE,
	//	[audioSystem] (iObject* self, cContext* context, XDataBuffer* data) {
	//		audioSystem->OnFrameUpdate();
	//	}
	//);

	// Create texture manager
	//cTextureAtlas* texture = _context->GetSubsystem<cTextureAtlas>();
	//texture->SetAtlas(glm::vec3(2048, 2048, 16));

	// Create sound context
	//cAudio* audio = _context->GetSubsystem<cAudio>();
}

void triton::cEngine::Shutdown()
{
	_context->GetSubsystem<cGraphics>()->Shutdown();
	_context->GetSubsystem<cTextureAtlas>()->Shutdown();
	_context->GetSubsystem<cInput>()->Shutdown();
	_context->GetSubsystem<XRenderSubsystem>()->Shutdown();
}

void triton::cEngine::Run()
{
	_context->GetSubsystem<XRenderSubsystem>()->MainThreadFunction(_app);
}