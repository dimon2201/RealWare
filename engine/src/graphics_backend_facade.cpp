// graphics_backend_facade.cpp

#include "graphics_backend_facade.hpp"
#include "context.hpp"

triton::cGraphicsBackendFacade::cGraphicsBackendFacade(cContext* context, iGraphicsBufferBackend* bufferBackend)
	: iObject(context), _buffer(bufferBackend) {}