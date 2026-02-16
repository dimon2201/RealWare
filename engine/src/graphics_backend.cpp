// graphics_backend.cpp

#include "graphics_backend.hpp"

triton::cRenderPassGPU::cRenderPassGPU(cContext* context, cVertexArray* vertexArray, cShader* shader)
	: iObject(context), _vertexArray(vertexArray), _shader(shader) {}

triton::iGraphicsBackend::iGraphicsBackend(cContext* context) : iBackend(context) {}