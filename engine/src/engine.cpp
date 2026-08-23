// engine.cpp

#include <tracy/Tracy.hpp>
#include "engine.hpp"
#include "application.hpp"
#include "context.hpp"
#include "graphics.hpp"
#include "texture_atlas.hpp"
#include "filesystem_manager.hpp"
#include "font_manager.hpp"
#include "physics_manager.hpp"
#include "time.hpp"
#include "event_manager.hpp"
#include "thread_subsystem.hpp"
#include "audio.hpp"
#include "math.hpp"
#include "input_backend_sdl.hpp"
#include "graphics_backend_ogl.hpp"
#include "audio_backend_oal.hpp"
#include "render_thread.hpp"
#include "thread_guard.hpp"
#include "geometry_storage.hpp"
#include "model3d_backend.hpp"
#include "model3d_backend_assimp.hpp"
#include "animation_pool.hpp"
#include "render_instance_pack_pool.hpp"
#include "synchronization.hpp"
#include "render_thread.hpp"
#include "camera_pool.hpp"
#include "skinned_bones_pool.hpp"
#include "game_object_pool.hpp"
#include "model3d_pool.hpp"
#include "skin_pool.hpp"
#include "animation_pool.hpp"
#include "input_layout_pool.hpp"
#include "render_target_pool.hpp"
#include "render_pass_pools.hpp"
#include "atlas_texture_pool.hpp"
#include "shader_pool.hpp"
#include "gpu_buffer_pool.hpp"
#include "render_instance_static_pool.hpp"
#include "render_instance_dynamic_pool.hpp"
#include "texture_pool.hpp"
#include "window.hpp"

using namespace types;

triton::CEngine::CEngine(cContext* context, IApplication* app) : CSubsystem(context), _app(app)
{
	Initialize();
}

triton::CEngine::~CEngine()
{
	Shutdown();
}

void triton::CEngine::Initialize()
{
	CThreadGuard::CaptureMainThreadId();

	// Register Engine as subsystem
	_context->RegisterSubsystem(this);

	// Create memory allocator
	_context->CreateMemoryAllocator();

	CObjectAllocator::Initialize(_context->GetMemoryAllocator());

	// Register backends
	_context->RegisterBackend<IInputBackend>(new CInputBackendSDL(_context));
	_context->RegisterBackend<IGraphicsBackend>(new XGraphicsBackendOGL(_context));
	_context->RegisterBackend<iAudioBackend>(new cAudioBackendOAL(_context));
	_context->RegisterBackend<IModel3DBackend>(new XModel3DBackendAssimp(_context));

	// Create application window
	_app->CreateWindow();

	// Initialize render thread
	InitializeRenderCommandRecorder();
	InitializeSynchronization();
	InitializeRenderThread();

	// Register subsystems and pools (order matters)
	_context->RegisterPool(new CCameraPool(_context, K_TRUE));

	_context->RegisterPool(new PTexturePool(_context, K_TRUE));

	_context->RegisterPool(new CShaderPool(_context, K_TRUE));

	_context->RegisterPool(new CInputLayoutPool(_context, K_TRUE));

	_context->RegisterPool(new CGPUBufferPool(_context, K_TRUE));

	_context->RegisterPool(new CRenderPassGeometryPool(_context, K_TRUE));

	_context->RegisterPool(new CRenderPassProcessingPool(_context, K_TRUE));

	_context->RegisterPool(new CRenderTargetPool(_context, K_TRUE));

	_context->RegisterPool(new CRenderInstancePackPool(_context, K_TRUE));

	_context->RegisterPool(new CRenderInstanceStaticPool(
		_context,
		K_TRUE,
		CRenderInstanceStaticPool::cDefaultReservationSize,
		0,
		EGPUBufferType::Storage
	));

	_context->RegisterPool(new CRenderInstanceDynamicPool(
		_context,
		K_TRUE,
		CRenderInstanceDynamicPool::cDefaultReservationSize,
		1,
		EGPUBufferType::Storage
	));

	_context->RegisterPool(new CAnimationPool(_context, K_TRUE));

	_context->RegisterPool(new CGameObjectPool(_context, K_TRUE));

	_context->RegisterPool(new CMaterialPool(
		_context,
		K_TRUE,
		CMaterialPool::cDefaultReservationSize,
		2,
		EGPUBufferType::Storage
	));

	_context->RegisterPool(new CModel3DPool(_context, K_TRUE));

	_context->RegisterPool(new CSkeletonPool(_context, K_TRUE));

	_context->RegisterPool(new CSkinnedBonesPool(
		_context,
		K_TRUE,
		CSkinnedBonesPool::cDefaultReservationSize,
		3,
		EGPUBufferType::Storage
	));

	_context->RegisterPool(new CSkinPool(_context, K_TRUE));

	_context->RegisterSubsystem(new CTextureAtlas(_context, cVector3(8193, 8193, 4)));

	_context->RegisterSubsystem(new CGeometryStorage(_context));

	_context->RegisterSubsystem(new CFileSystem(_context));

	_context->RegisterSubsystem(new CGraphics(_context));
}

void triton::CEngine::Shutdown()
{
	ReleaseRenderThread();
	ReleaseSynchronization();
	ReleaseRenderCommandRecorder();
}

void triton::CEngine::Run()
{
	MainThreadFunction();
}

void triton::CEngine::InitializeSynchronization()
{
	_sync = _context->Create<XSynchronization>(_context, _cmdRecorder, 1024);
}

void triton::CEngine::InitializeRenderCommandRecorder()
{
	CThreadGuard::AssertMain();

	_cmdRecorder = _context->Create<XRenderCommandRecorder>();
}

void triton::CEngine::InitializeRenderThread()
{
	CThreadGuard::AssertMain();

	// Create render thread
	_renderThread = _context->Create<cRenderThread>(_context, _sync, _app->GetWindow());
	_renderThread->Run();

	// Wait until render thread gets initialized
	_sync->WaitForRenderThreadToInit();
}

void triton::CEngine::ReleaseSynchronization()
{
	_context->Destroy<XSynchronization>(_sync);
}

void triton::CEngine::ReleaseRenderCommandRecorder()
{
	_context->Destroy<XRenderCommandRecorder>(_cmdRecorder);
}

void triton::CEngine::ReleaseRenderThread()
{
	_renderThread->Stop();
	_context->Destroy<cRenderThread>(_renderThread);
}

void triton::CEngine::MainThreadFunction()
{
	CThreadGuard::AssertMain();

	std::cout << "Main thread started\n";

	tracy::SetThreadName("Main Thread");

	if (!_app)
		return;

	_app->Setup();

	boolean bIsRunning = True;

	IInputBackend* inputBackend = _context->GetBackend<IInputBackend>();
	CWindow* window = _app->GetWindow();

	while (bIsRunning)
	{
		FrameMark;

		EProducedFrameOp producedFrameOp = EProducedFrameOp::ExecuteFull;
		SWindowEvent e = {};

		inputBackend->PreparePollEvent();

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
				inputBackend->ProcessEvent(e, window->GetBackendWindow());
			}
		}

		if (e.type != EWindowEvent::Quit)
		{
			ZoneScopedN("Main Job");

			_app->Update();

			_context->GetPool<CRenderInstanceStaticPool>()->Update();
			_context->GetPool<CRenderInstanceDynamicPool>()->Update();
			_context->GetPool<CMaterialPool>()->Update();
			_context->GetPool<CSkinnedBonesPool>()->Update();
		}
		
		{
			ZoneScopedN("Wait for Released Frame");

			_sync->WaitForReleasedFrame();
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