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
    : iGraphicsBackend(context), _buffer(bufferBackend) {}*/