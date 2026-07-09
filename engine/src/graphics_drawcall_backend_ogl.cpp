// graphics_drawcall_backend_ogl.cpp

#include "gl.hpp"
#include "graphics_drawcall_backend_ogl.hpp"

using namespace types;

triton::cGraphicsDrawcallBackendOGL::cGraphicsDrawcallBackendOGL(cContext* context)
	: iGraphicsDrawcallBackend(context) {}

void triton::cGraphicsDrawcallBackendOGL::ClearColor(const cVector4& color)
{
    glDisable(GL_CULL_FACE);

    glClearColor(color.GetX(), color.GetY(), color.GetZ(), color.GetW());
    glClear(GL_COLOR_BUFFER_BIT);
}

void triton::cGraphicsDrawcallBackendOGL::ClearDepth(const f32 depth)
{
    glClearDepth(depth);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void triton::cGraphicsDrawcallBackendOGL::ClearFramebufferColor(usize bufferIndex, const cVector4& color)
{
    GLfloat buff[] = { color.GetX(), color.GetY(), color.GetZ(), color.GetW() };
    glClearBufferfv(GL_COLOR, bufferIndex, &buff[0]);
}

void triton::cGraphicsDrawcallBackendOGL::ClearFramebufferDepth(f32 depth)
{
    glClearDepth(depth);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void triton::cGraphicsDrawcallBackendOGL::Draw(
    usize indexCount,
    usize vertexElementOffset,
    usize indexElementOffset,
    usize instanceCount
)
{
    glDrawElementsInstancedBaseVertex(
        GL_TRIANGLES,
        indexCount,
        GL_UNSIGNED_INT,
        (const void*)(indexElementOffset * sizeof(u32)),
        instanceCount,
        vertexElementOffset
    );
}

void triton::cGraphicsDrawcallBackendOGL::DrawQuad()
{
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void triton::cGraphicsDrawcallBackendOGL::DrawQuads(usize count)
{
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, count);
}