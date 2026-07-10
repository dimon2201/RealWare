// graphics_context_backend_ogl.cpp

#include <glbinding/gl/gl.h>
#include <SDL3/SDL.h>
#include "graphics_context_backend_ogl.hpp"
#include "log.hpp"

using namespace gl;
using namespace types;

namespace triton
{
    static void GLDebugCallback(
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
}

triton::cGraphicsContextBackendOGL::cGraphicsContextBackendOGL(cContext* context)
	: iGraphicsContextBackend(context) {}

void triton::cGraphicsContextBackendOGL::CreateGraphicsContext(sInputBackendWindow& window)
{
    window.renderContextInstance = (qword)SDL_GL_CreateContext((SDL_Window*)window.instance);
    if (!window.renderContextInstance)
    {
        Print("Error: Failed to create GL context");
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

void triton::cGraphicsContextBackendOGL::MakeWindowGraphicsContextCurrent(sInputBackendWindow& window)
{
    SDL_GL_MakeCurrent((SDL_Window*)window.instance, (SDL_GLContext)window.renderContextInstance);
}

void triton::cGraphicsContextBackendOGL::SwapWindowBuffers(sInputBackendWindow& window)
{
    SDL_GL_SwapWindow((SDL_Window*)window.instance);
}