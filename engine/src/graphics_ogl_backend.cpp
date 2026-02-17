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

triton::cVertexArray* triton::cGraphicsOGLBackend::CreateVertexArray()
{
    cVertexArray* vertexArray = _context->Create<cVertexArray>(_context);

    glGenVertexArrays(1, (GLuint*)&vertexArray->_instance);

    return vertexArray;
}

void triton::cGraphicsOGLBackend::BindVertexArray(const cVertexArray* vertexArray)
{
    glBindVertexArray((GLuint)vertexArray->_instance);
}

void triton::cGraphicsOGLBackend::BindDefaultVertexArray(const std::vector<cBuffer*>& buffersToBind)
{
    static cVertexArray* vertexArray = nullptr;

    if (vertexArray == nullptr)
    {
        vertexArray = CreateVertexArray();

        BindVertexArray(vertexArray);
        for (auto buffer : buffersToBind)
            _buffer->BindBuffer(buffer);
        BindDefaultInputLayout();
        UnbindVertexArray();
    }

    BindVertexArray(vertexArray);
}

void triton::cGraphicsOGLBackend::UnbindVertexArray()
{
    glBindVertexArray(0);
}

void triton::cGraphicsOGLBackend::DestroyVertexArray(cVertexArray* vertexArray)
{
    glDeleteVertexArrays(1, (GLuint*)&vertexArray->_instance);

    if (vertexArray != nullptr)
        _context->Destroy<cVertexArray>(vertexArray);
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

triton::cRenderPassGPU* triton::cGraphicsOGLBackend::CreateRenderPass(const sRenderPassDescriptor& desc)
{
    std::vector<cShader::sDefinePair> definePairs = {};
    cVertexArray* vertexArray = nullptr;
    cShader* shader = nullptr;

    if (desc.inputTextureAtlasTextures.size() != desc.inputTextureAtlasTextureNames.size())
    {
        Print("Error: mismatch of render pass input texture atlas texture array and input texture atlas texture name array!");
        return nullptr;
    }
    for (usize i = 0; i < desc.inputTextureAtlasTextures.size(); i++)
    {
        const usize textureAtlasTextureIndex = i;
        const std::string& textureAtlasTextureName = desc.inputTextureAtlasTextureNames[i];
        definePairs.push_back({ textureAtlasTextureName, textureAtlasTextureIndex });
    }

    if (desc.shaderBase == nullptr)
    {
        shader = CreateShader(
            desc.shaderRenderPath,
            desc.shaderVertexPath,
            desc.shaderFragmentPath,
            definePairs
        );
    }
    else
    {
        shader = CreateShader(
            desc.shaderBase,
            desc.shaderVertexFunc,
            desc.shaderFragmentFunc,
            definePairs
        );
    }

    vertexArray = CreateVertexArray();
    BindVertexArray(vertexArray);
    if (desc.inputVertexFormat == eCategory::VERTEX_BUFFER_FORMAT_NONE)
    {
        for (auto buffer : desc.inputBuffers)
            _buffer->BindBuffer(buffer);
    }
    else if (desc.inputVertexFormat == eCategory::VERTEX_BUFFER_FORMAT_POS_TEX_NRM_VEC3_VEC2_VEC3)
    {
        for (auto buffer : desc.inputBuffers)
            _buffer->BindBuffer(buffer);

        BindDefaultInputLayout();
    }

    UnbindVertexArray();

    return _context->Create<cRenderPassGPU>(_context, vertexArray, shader);
}

void triton::cGraphicsOGLBackend::BindRenderPass(const cRenderPass* renderPass, cShader* customShader)
{
    cShader* shader = nullptr;
    if (customShader == nullptr)
        shader = renderPass->GetRenderPassGPU()->GetShader();
    else
        shader = customShader;

    BindShader(shader);
    BindVertexArray(renderPass->GetRenderPassGPU()->GetVertexArray());
    if (renderPass->GetRenderTarget() != nullptr)
        BindRenderTarget(renderPass->GetRenderTarget());
    else
        UnbindRenderTarget();
    Viewport(renderPass->GetViewport());
    for (auto buffer : renderPass->GetInputBuffers())
        _buffer->BindBufferNotVAO(buffer);
    BindDepthMode(renderPass->GetDepthMode());
    BindBlendMode(renderPass->GetBlendMode());
    for (usize i = 0; i < renderPass->GetInputTextures().size(); i++)
        BindTexture(shader, renderPass->GetInputTextureNames()[i].c_str(), renderPass->GetInputTextures()[i], i);
}

void triton::cGraphicsOGLBackend::UnbindRenderPass(const cRenderPass* renderPass)
{
    UnbindVertexArray();
    if (renderPass->GetRenderTarget() != nullptr)
        UnbindRenderTarget();
    for (auto buffer : renderPass->GetInputBuffers())
        _buffer->UnbindBuffer(buffer);
    for (auto texture : renderPass->GetInputTextures())
        UnbindTexture(texture);
}

void triton::cGraphicsOGLBackend::DestroyRenderPass(cRenderPassGPU* renderPass)
{
    glBindVertexArray(0);
    DestroyVertexArray(renderPass->GetVertexArray());
        
    DestroyShader(renderPass->GetShader());
        
    _context->Destroy<cRenderPassGPU>(renderPass);
}

void triton::cGraphicsOGLBackend::BindDefaultInputLayout()
{
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 32, (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 32, (void*)12);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 32, (void*)20);
}

void triton::cGraphicsOGLBackend::BindBlendMode(const sBlendMode& blendMode)
{
    for (usize i = 0; i < blendMode.factorCount; i++)
    {
        GLuint srcFactor = GL_ZERO;
        GLuint dstFactor = GL_ZERO;

        switch (blendMode.srcFactors[i])
        {
            case sBlendMode::eFactor::ONE: srcFactor = GL_ONE; break;
            case sBlendMode::eFactor::SRC_COLOR: srcFactor = GL_SRC_COLOR; break;
            case sBlendMode::eFactor::INV_SRC_COLOR: srcFactor = GL_ONE_MINUS_SRC_COLOR; break;
            case sBlendMode::eFactor::SRC_ALPHA: srcFactor = GL_SRC_ALPHA; break;
            case sBlendMode::eFactor::INV_SRC_ALPHA: srcFactor = GL_ONE_MINUS_SRC_ALPHA; break;
        }

        switch (blendMode.dstFactors[i])
        {
            case sBlendMode::eFactor::ONE: dstFactor = GL_ONE; break;
            case sBlendMode::eFactor::SRC_COLOR: dstFactor = GL_SRC_COLOR; break;
            case sBlendMode::eFactor::INV_SRC_COLOR: dstFactor = GL_ONE_MINUS_SRC_COLOR; break;
            case sBlendMode::eFactor::SRC_ALPHA: dstFactor = GL_SRC_ALPHA; break;
            case sBlendMode::eFactor::INV_SRC_ALPHA: dstFactor = GL_ONE_MINUS_SRC_ALPHA; break;
        }

        glBlendFunci(i, srcFactor, dstFactor);
    }
}

void triton::cGraphicsOGLBackend::BindDepthMode(const sDepthMode& blendMode)
{
    if (blendMode.useDepthTest == K_TRUE)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);

    if (blendMode.useDepthWrite == K_TRUE)
        glDepthMask(GL_TRUE);
    else
        glDepthMask(GL_FALSE);
}

void triton::cGraphicsOGLBackend::Viewport(const sViewport& viewport)
{
    glViewport(viewport.rect.GetX(), viewport.rect.GetY(), viewport.rect.GetZ(), viewport.rect.GetW());
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