// render_thread.cpp

#include "render_thread.hpp"
#include "context.hpp"
#include "input.hpp"
#include "texture_manager.hpp"
#include "graphics.hpp"

triton::cRenderThread::cRenderThread(cContext* context) : cThread(context)
{
}

void triton::cRenderThread::ThreadFunction()
{
	// Initialize graphics-related subsystems
	// NOTE: order matters
	_context->GetSubsystem<cInput>()->Initialize();
	_context->GetSubsystem<cTextureAtlas>()->Initialize(cVector3(2048, 2048, 16));
	_context->GetSubsystem<cGraphics>()->Initialize();
}