// platform.cpp

#include "platform.hpp"
#include "input_glfw_backend.hpp"
#include "render_context.hpp"

triton::cPlatform::cPlatform(cContext* context, eInputBackend inputBackend, eGraphicsBackend graphicsBackend) 
	: iObject(context)
{
	if (inputBackend == eInputBackend::GLFW)
		RegisterBackend<iInputBackend>(new cInputGLFWBackend(_context));

	if (graphicsBackend == eGraphicsBackend::OGL)
		RegisterBackend<iGraphicsBackend>(new cOGLGraphicsBackend(_context));
}