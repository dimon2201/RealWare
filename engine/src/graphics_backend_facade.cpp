// graphics_backend_facade.cpp

#include "graphics_backend_facade.hpp"
#include "context.hpp"

triton::cGraphicsBackendFacade::cGraphicsBackendFacade(
	cContext* context,
    iGraphicsResourceBackend* resourceBackend,
    iGraphicsPipelineBackend* pipelineBackend,
    iGraphicsContextBackend* contextBackend,
    iGraphicsDrawcallBackend* drawcallBackend
)
: iObject(context),
_resource(resourceBackend),
_pipeline(pipelineBackend),
_context(contextBackend),
_drawcall(drawcallBackend) {}