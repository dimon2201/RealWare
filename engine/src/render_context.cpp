// render_context.cpp

#include "render_context.hpp"

triton::cRenderPassGPU::cRenderPassGPU(cContext* context, cVertexArray* vertexArray, cShader* shader)
	: iObject(context), _vertexArray(vertexArray), _shader(shader) {}