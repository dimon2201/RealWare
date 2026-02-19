// graphics_context_backend_ogl.cpp

#include <GL/glew.h>
#include "graphics_context_backend_ogl.hpp"
#include "log.hpp"

namespace triton
{
    void GLAPIENTRY GLDebugCallback(
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

void triton::cGraphicsContextBackendOGL::BindWindowContext(void* nativeWindow)
{
    //glfwMakeContextCurrent((GLFWwindow*)nativeWindow);
}

void triton::cGraphicsContextBackendOGL::CreateGraphicsContext()
{
    if (glewInit() != GLEW_OK)
    {
        Print("Error: can't initialize OpenGL 4.3 context!");
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
    glDebugMessageCallback(triton::GLDebugCallback, nullptr);
}