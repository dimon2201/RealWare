// graphics_debug_callback.hpp

#pragma once

#include <GL/glew.h>
#include "log.hpp"

namespace triton
{
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
}