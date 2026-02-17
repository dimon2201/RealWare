// graphics_backend_facade.cpp

#include "graphics_backend_facade.hpp"
#include "context.hpp"

triton::cRenderPassGPU::cRenderPassGPU(cContext* context, cVertexArray* vertexArray, cShader* shader)
	: iObject(context), _vertexArray(vertexArray), _shader(shader) {}

triton::cGraphicsBackendFacade::cGraphicsBackendFacade(cContext* context, iGraphicsBufferBackend* bufferBackend)
	: iObject(context), _buffer(bufferBackend) {}