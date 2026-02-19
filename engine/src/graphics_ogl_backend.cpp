// graphics_ogl_backend.cpp

#include <iostream>
#include <cstring>
#include <string>
#include <lodepng.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "buffer.hpp"
#include "graphics_ogl_backend.hpp"
#include "filesystem_manager.hpp"
#include "types.hpp"
#include "application.hpp"
#include "memory_pool.hpp"
#include "context.hpp"
#include "engine.hpp"
#include "graphics.hpp"
#include "input_backend.hpp"
#include "log.hpp"

using namespace types;

void APIENTRY GLDebugCallback(
    GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    const void* userParam
)
{
    triton::Print(message);
}

std::string CleanShaderSource(const std::string& src)
{
    std::string out;
    out.reserve(src.size());
    for (u8 c : src)
    {
        if (c == '\t' || c == '\n' || c == '\r' || (c >= 32 && c <= 126))
            out.push_back(c);
    }

    return out;
}

/*triton::cGraphicsOGLBackend::cGraphicsOGLBackend(cContext* context, iGraphicsBufferBackend* bufferBackend)
    : iGraphicsBackend(context), _buffer(bufferBackend) {}

void triton::cGraphicsOGLBackend::BindWindowContext(void* nativeWindow)
{
    glfwMakeContextCurrent((GLFWwindow*)nativeWindow);
}

void triton::cGraphicsOGLBackend::CreateGraphicsContext()
{
    if (glewInit() != GLEW_OK)
    {
        Print("Error: can't initialize GL context!");
        return;
    }

    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glDepthFunc(GL_LESS);
    //glCullFace(GL_BACK);
    glFrontFace(GL_CW);
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(GLDebugCallback, nullptr);
}

void triton::cGraphicsOGLBackend::ClearColor(const glm::vec4& color)
{
    glClearColor(color.x, color.y, color.z, color.w);
    glClear(GL_COLOR_BUFFER_BIT);
}

void triton::cGraphicsOGLBackend::ClearDepth(const f32 depth)
{
    glClearDepth(depth);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void triton::cGraphicsOGLBackend::ClearFramebufferColor(usize bufferIndex, const glm::vec4& color)
{
    glClearBufferfv(GL_COLOR, bufferIndex, &color.x);
}

void triton::cGraphicsOGLBackend::ClearFramebufferDepth(f32 depth)
{
    glClearDepth(depth);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void triton::cGraphicsOGLBackend::Draw(usize indexCount, usize vertexOffset, usize indexOffset, usize instanceCount)
{
    glDrawElementsInstancedBaseVertex(
        GL_TRIANGLES,
        indexCount,
        GL_UNSIGNED_INT,
        (const void*)indexOffset,
        instanceCount,
        vertexOffset
    );
}

void triton::cGraphicsOGLBackend::DrawQuad()
{
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void triton::cGraphicsOGLBackend::DrawQuads(usize count)
{
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, count);
}*/