// render_thread.cpp

#include "render_thread.hpp"
#include "context.hpp"
#include "input.hpp"
#include "texture_manager.hpp"
#include "graphics.hpp"
#include "graphics_context_backend.hpp"

using namespace types;

triton::cRenderThread::cRenderThread(cContext* context, cEngine* engine) : cThread(context), _engine(engine)
{
	_initialized.store(K_FALSE);
}

void triton::cRenderThread::ThreadFunction()
{
	// Create graphics contexts for windows
	cInput* inputSubsystem = _context->GetSubsystem<cInput>();
	cStack<cInputWindow>* windows = inputSubsystem->GetWindows();
	iGraphicsContextBackend* gfxContextBackend = _context->GetBackend<iGraphicsContextBackend>();
	for (usize i = 0; i < windows->GetSize(); i++)
	{
		gfxContextBackend->MakeWindowGraphicsContextCurrent(windows->At(i)->GetBackendWindow());
		gfxContextBackend->CreateGraphicsContext();
	}

	_initialized.store(K_TRUE);

	_engine->NotifyThread();

	while (K_TRUE)
	{
		//gfx->CompositeFinal();
		//window->SwapBuffers();
	}

	// Initialize graphics-related subsystems
	// NOTE: order matters
	//_context->GetSubsystem<cTextureAtlas>()->Initialize(cVector3(2048, 2048, 16));
	//_context->GetSubsystem<cGraphics>()->Initialize();
}