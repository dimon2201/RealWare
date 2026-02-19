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

triton::cRenderTarget* triton::cGraphicsOGLBackend::CreateRenderTarget(const std::vector<cTexture*>& colorAttachments, cTexture* depthAttachment)
{
    cRenderTarget* renderTarget = _context->Create<cRenderTarget>(_context);

    renderTarget->_colorAttachments = colorAttachments;
    renderTarget->_depthAttachment = depthAttachment;

    GLenum buffs[16] = {};
    glGenFramebuffers(1, (GLuint*)&renderTarget->_instance);
    glBindFramebuffer(GL_FRAMEBUFFER, renderTarget->_instance);
    for (usize i = 0; i < renderTarget->_colorAttachments.size(); i++)
    {
        buffs[i] = GL_COLOR_ATTACHMENT0 + i;
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, renderTarget->_colorAttachments[i]->_instance, 0);
    }
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, renderTarget->_depthAttachment->_instance, 0);
    glDrawBuffers(renderTarget->_colorAttachments.size(), &buffs[0]);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
			
	if (status != GL_FRAMEBUFFER_COMPLETE)
        Print("Error: incomplete framebuffer!");

    return renderTarget;
}

void triton::cGraphicsOGLBackend::ResizeRenderTargetColors(cRenderTarget* renderTarget, const glm::vec2& size)
{
    std::vector<cTexture*> newColorAttachments;
    for (auto attachment : renderTarget->_colorAttachments)
    {
        newColorAttachments.emplace_back(CreateTexture(size.x, size.y, attachment->GetDepth(), attachment->GetDimension(), attachment->GetFormat(), nullptr));
        DestroyTexture(attachment);
    }
    renderTarget->_colorAttachments.clear();
    renderTarget->_colorAttachments = newColorAttachments;

    GLenum buffs[16] = {};
    glBindFramebuffer(GL_FRAMEBUFFER, renderTarget->_instance);
    for (usize i = 0; i < renderTarget->_colorAttachments.size(); i++)
    {
        buffs[i] = GL_COLOR_ATTACHMENT0 + i;
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, renderTarget->_colorAttachments[i]->_instance, 0);
    }
    glDrawBuffers(renderTarget->_colorAttachments.size(), &buffs[0]);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void triton::cGraphicsOGLBackend::ResizeRenderTargetDepth(cRenderTarget* renderTarget, const glm::vec2& size)
{
    cTexture* newDepthAttachment = CreateTexture(size.x, size.y, renderTarget->_depthAttachment->GetDepth(), renderTarget->_depthAttachment->GetDimension(), renderTarget->_depthAttachment->GetFormat(), nullptr);
    DestroyTexture(renderTarget->_depthAttachment);
    renderTarget->_depthAttachment = newDepthAttachment;

    GLenum buffs[16] = {};
    glBindFramebuffer(GL_FRAMEBUFFER, renderTarget->_instance);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, renderTarget->_depthAttachment->_instance, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void triton::cGraphicsOGLBackend::UpdateRenderTargetBuffers(cRenderTarget* renderTarget)
{
    GLenum buffs[16] = {};
    glGenFramebuffers(1, (GLuint*)&renderTarget->_instance);
    glBindFramebuffer(GL_FRAMEBUFFER, renderTarget->_instance);
    for (usize i = 0; i < renderTarget->_colorAttachments.size(); i++)
    {
        buffs[i] = GL_COLOR_ATTACHMENT0 + i;
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, renderTarget->_colorAttachments[i]->_instance, 0);
    }
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, renderTarget->_depthAttachment->_instance, 0);
    glDrawBuffers(renderTarget->_colorAttachments.size(), &buffs[0]);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void triton::cGraphicsOGLBackend::BindRenderTarget(const cRenderTarget* renderTarget)
{
    glBindFramebuffer(GL_FRAMEBUFFER, renderTarget->_instance);
}

void triton::cGraphicsOGLBackend::UnbindRenderTarget()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void triton::cGraphicsOGLBackend::DestroyRenderTarget(cRenderTarget* renderTarget)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, (GLuint*)&renderTarget->_instance);

    if (renderTarget != nullptr)
        _context->Destroy<cRenderTarget>(renderTarget);
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