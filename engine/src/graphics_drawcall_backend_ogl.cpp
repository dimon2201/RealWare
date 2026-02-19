// graphics_drawcall_backend_ogl.cpp

#include <GL/glew.h>
#include "graphics_drawcall_backend_ogl.hpp"

using namespace types;

triton::cGraphicsDrawcallBackendOGL::cGraphicsDrawcallBackendOGL(cContext* context)
	: iGraphicsDrawcallBackend(context) {}