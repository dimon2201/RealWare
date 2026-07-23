// engine.cpp

#include <tracy/Tracy.hpp>
#include "engine.hpp"
#include "application.hpp"
#include "context.hpp"
#include "graphics.hpp"
#include "input.hpp"
#include "texture_subsystem.hpp"
#include "filesystem_manager.hpp"
#include "font_manager.hpp"
#include "physics_manager.hpp"
#include "time.hpp"
#include "event_manager.hpp"
#include "thread_subsystem.hpp"
#include "audio.hpp"
#include "math.hpp"
#include "ecs.hpp"
#include "input_backend_sdl.hpp"
#include "graphics_resource_backend_ogl.hpp"
#include "graphics_pipeline_backend_ogl.hpp"
#include "graphics_context_backend_ogl.hpp"
#include "graphics_drawcall_backend_ogl.hpp"
#include "audio_backend_oal.hpp"
#include "render_thread.hpp"
#include "thread_guard.hpp"
#include "geometry_storage.hpp"
#include "ecs_subsystem.hpp"
#include "instance_buffer.hpp"
#include "handle_allocator.hpp"
#include "game_object_subsystem.hpp"
#include "material_subsystem.hpp"
#include "model3d_backend.hpp"
#include "model3d_backend_assimp.hpp"
#include "model3d_subsystem.hpp"
#include "animation_subsystem.hpp"
#include "skeleton_subsystem.hpp"
#include "skinning_subsystem.hpp"
#include "batcher.hpp"
#include "synchronization.hpp"
#include "render_thread.hpp"

using namespace triton::ecs;
using namespace triton::ecs::components;
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

	// Register Engine as subsystem
	_context->RegisterSubsystem(this);

	// Create memory allocator
	_context->CreateMemoryAllocator();

	InitializeRenderCommandRecorder();
	InitializeSynchronization();
	
	CObjectAllocator::Initialize(_context->GetMemoryAllocator());

	// Register backends
	_context->RegisterBackend<iInputBackend>(new cInputBackendSDL(_context));
	_context->RegisterBackend<iGraphicsResourceBackend>(new cGraphicsResourceBackendOGL(_context));
	_context->RegisterBackend<iGraphicsPipelineBackend>(new cGraphicsPipelineBackendOGL(_context));
	_context->RegisterBackend<iGraphicsContextBackend>(new cGraphicsContextBackendOGL(_context));
	_context->RegisterBackend<iGraphicsDrawcallBackend>(new cGraphicsDrawcallBackendOGL(_context));
	_context->RegisterBackend<iAudioBackend>(new cAudioBackendOAL(_context));
	_context->RegisterBackend<IModel3DBackend>(new XModel3DBackendAssimp(_context));
	
	// Register subsystems (order matters)
	_context->RegisterSubsystem(new cInput(_context));
	_context->GetSubsystem<cInput>()->Initialize();

	InitializeRenderThread();

	//_context->RegisterSubsystem(new cAudio(_context));
	_context->RegisterSubsystem(new XTextureSubsystem(_context, cVector3(8193, 8193, 4)));
	_context->RegisterSubsystem(new XMaterialSubsystem(_context));
	_context->RegisterSubsystem(new cFileSystem(_context));
	//_context->RegisterSubsystem(new cTime(_context));
	//_context->RegisterSubsystem(new cThreadSubsystem(_context));
	_context->RegisterSubsystem(new XECSSubsystem(_context));
	_context->GetSubsystem<XECSSubsystem>()->Initialize();
	_context->RegisterSubsystem(new cGraphics(_context));
	_context->GetSubsystem<cGraphics>()->Init();
	_context->RegisterSubsystem(new XGameObjectSubsystem(_context));
	_context->GetSubsystem<XGameObjectSubsystem>()->Init();
	_context->RegisterSubsystem(new XModel3DSubsystem(_context));
	_context->RegisterSubsystem(new XAnimationSubsystem(_context));
	_context->GetSubsystem<XAnimationSubsystem>()->Init();
	_context->RegisterSubsystem(new XSkeletonSubsystem(_context));
	_context->GetSubsystem<XSkeletonSubsystem>()->Init();
	_context->RegisterSubsystem(new XSkinningSubsystem(_context));
	_context->GetSubsystem<XSkinningSubsystem>()->Init();
	_context->RegisterSubsystem(new XBatchSubsystem(_context));
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
	ReleaseRenderThread();
	ReleaseSynchronization();
	ReleaseRenderCommandRecorder();
	_context->GetSubsystem<cInput>()->Shutdown();
	_context->GetSubsystem<XECSSubsystem>()->Shutdown();
}

void triton::cEngine::Run()
{
	MainThreadFunction();
}

void triton::cEngine::InitializeSynchronization()
{
	_sync = _context->Create<XSynchronization>(_context, _cmdRecorder);
}

void triton::cEngine::InitializeRenderCommandRecorder()
{
	CThreadGuard::AssertMain();

	_cmdRecorder = _context->Create<XRenderCommandRecorder>(_context);
}

void triton::cEngine::InitializeRenderThread()
{
	CThreadGuard::AssertMain();

	// Create render thread
	_renderThread = _context->Create<cRenderThread>(_context, _sync);
	_renderThread->Run();

	// Wait until render thread gets initialized
	_sync->WaitForRenderThreadToInit();
}

void triton::cEngine::ReleaseSynchronization()
{
	_context->Destroy<XSynchronization>(_sync);
}

void triton::cEngine::ReleaseRenderCommandRecorder()
{
	_context->Destroy<XRenderCommandRecorder>(_cmdRecorder);
}

void triton::cEngine::ReleaseRenderThread()
{
	_renderThread->Stop();
	_context->Destroy<cRenderThread>(_renderThread);
}

void triton::cEngine::MainThreadFunction()
{
	CThreadGuard::AssertMain();

	std::cout << "Main thread started\n";

	tracy::SetThreadName("Main Thread");

	if (!_app)
		return;

	_app->Setup();

	iInputBackend* inputBackend = _context->GetBackend<iInputBackend>();
	cInputWindow& window = _context->GetSubsystem<cInput>()->GetWindows()->at(0);
	boolean bIsRunning = K_TRUE;

	while (bIsRunning)
	{
		FrameMark;

		EProducedFrameOp producedFrameOp = EProducedFrameOp::ExecuteFull;
		SEvent e = {};
		while ((e = inputBackend->PollEvent()).type != EWindowEvent::None)
		{
			if (e.type == EWindowEvent::Quit)
			{
				bIsRunning = K_FALSE;
				producedFrameOp = EProducedFrameOp::Kill;
				break;
			}
			else
			{
				inputBackend->ProcessEvent(e);
			}
		}
		
		{
			ZoneScopedN("Wait for Released Frame");

			_sync->WaitForReleasedFrame();
		}

		if (e.type != EWindowEvent::Quit)
		{
			ZoneScopedN("Main Job");

			_app->Update();
			_context->GetSubsystem<XGameObjectSubsystem>()->Update();
			_context->GetSubsystem<XMaterialSubsystem>()->Update();
			_context->GetSubsystem<XTextureSubsystem>()->Update();
			_context->GetSubsystem<XAnimationSubsystem>()->Update();
			_context->GetSubsystem<XSkeletonSubsystem>()->Update();
			_context->GetSubsystem<XSkinningSubsystem>()->Update();
			_context->GetSubsystem<XBatchSubsystem>()->Update();
		}

		{
			ZoneScopedN("Produce Frame");

			_sync->ProduceFrame(
				producedFrameOp,
				_cmdRecorder->GetRenderCommandPack()
			);
		}
	}

	//input->DestroyWindow(&window);

	// Stop render thread
	// TODO: main thread must wait until render thread finishes job completely
	_renderThread->Stop();

	_app->Stop();
}