// graphics_backend_facade.cpp

#include "graphics_backend_facade.hpp"
#include "context.hpp"

triton::cGraphicsBackendFacade::cGraphicsBackendFacade(
	cContext* context,
	iGraphicsResourceBackend* resourceBackend
)
: iObject(context), _resource(resourceBackend) {}