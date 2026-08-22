// graphics_backend_ogl.cpp

#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>
#include <SDL3/SDL.h>
#include "graphics_backend_ogl.hpp"
#include "context.hpp"
#include "filesystem_manager.hpp"
#include "application.hpp"
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
        Print(message);
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

    void DefineInShader(std::string& shaderVertexStr, std::string& shaderFragmentStr, const std::vector<SShaderDefine>& definePairs)
    {
        if (!definePairs.empty())
        {
            std::string defineStr = "";
            for (const auto& define : definePairs)
                defineStr += "#define " + define.name + " " + std::to_string(define.index) + "\n";

            shaderVertexStr = defineStr + shaderVertexStr;
            shaderFragmentStr = defineStr + shaderFragmentStr;
        }
    }
}

void triton::XGraphicsBackendOGL::CreateGraphicsContext(SWindowBackend& window)
{
    window.renderContextInstance = (qword)SDL_GL_CreateContext((SDL_Window*)window.instance);
    if (!window.renderContextInstance)
    {
        Print("Error: Failed to create GL context");
        return;
    }

    glbinding::initialize(SDL_GL_GetProcAddress);

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

void triton::XGraphicsBackendOGL::MakeWindowGraphicsContextCurrent(const SWindowBackend& window)
{
    SDL_GL_MakeCurrent((SDL_Window*)window.instance, (SDL_GLContext)window.renderContextInstance);
}

void triton::XGraphicsBackendOGL::SwapWindowBuffers(const SWindowBackend& window)
{
    SDL_GL_SwapWindow((SDL_Window*)window.instance);
}

void triton::XGraphicsBackendOGL::ClearColor(const cVector4& color)
{
    glClearColor(color.GetX(), color.GetY(), color.GetZ(), color.GetW());
    glClear(GL_COLOR_BUFFER_BIT);
}

void triton::XGraphicsBackendOGL::ClearDepth(const f32 depth)
{
    glClearDepth(depth);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void triton::XGraphicsBackendOGL::ClearFramebufferColor(usize bufferIndex, const cVector4& color)
{
    GLfloat buff[] = { color.GetX(), color.GetY(), color.GetZ(), color.GetW() };
    glClearBufferfv(GL_COLOR, bufferIndex, &buff[0]);
}

void triton::XGraphicsBackendOGL::ClearFramebufferDepth(f32 depth)
{
    glClearDepth(depth);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void triton::XGraphicsBackendOGL::Draw(
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

void triton::XGraphicsBackendOGL::DrawQuad()
{
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void triton::XGraphicsBackendOGL::DrawQuads(usize count)
{
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, count);
}

triton::CGPUShaderResource triton::XGraphicsBackendOGL::CreateShader(
    const char* vertexStr,
    const char* fragmentStr,
    const char* vertexCustomFuncStr,
    const char* fragmentCustomFuncStr,
    types::usize defineCount,
    const SShaderDefine* defines,
    types::usize vertexIncludePathCount,
    const char** vertexIncludePaths,
    types::usize fragmentIncludePathCount,
    const char** fragmentIncludePaths
)
{
    std::string vertexCppStr = vertexStr;
    std::string fragmentCppStr = fragmentStr;

    const std::string appendStr = "#version 430\n\n";
    const std::string vertexFuncDefinition = "void Vertex_Func(in vec3 _positionLocal, in vec2 _texcoord, in vec3 _normal, in int _instanceID, in Instance _instance, in Material material, in float _use2D, out vec4 _glPosition){}";
    const std::string vertexFuncPassthroughCall = "Vertex_Passthrough(InPositionLocal, instance, instance.Use2D, gl_Position);";
    const std::string fragmentFuncDefinition = "void Fragment_Func(in vec2 _texcoord, in vec4 _textureColor, in vec4 _materialDiffuseColor, out vec4 _fragColor){}";
    const std::string fragmentFuncPassthroughCall = "Fragment_Passthrough(textureColor, DiffuseColor, fragColor);";

    if (!vertexCustomFuncStr && !fragmentCustomFuncStr)
    {
        const usize vertexFuncDefinitionPos = vertexCppStr.find(vertexFuncDefinition);
        if (vertexFuncDefinitionPos != std::string::npos)
            vertexCppStr.replace(vertexFuncDefinitionPos, vertexFuncDefinition.length(), vertexCustomFuncStr);
        const usize vertexFuncPasstroughCallPos = vertexCppStr.find(vertexFuncPassthroughCall);
        if (vertexFuncPasstroughCallPos != std::string::npos)
            vertexCppStr.replace(vertexFuncPasstroughCallPos, vertexFuncPassthroughCall.length(), "");
        const usize fragmentFuncDefinitionPos = fragmentCppStr.find(fragmentFuncDefinition);
        if (fragmentFuncDefinitionPos != std::string::npos)
            fragmentCppStr.replace(fragmentFuncDefinitionPos, fragmentFuncDefinition.length(), fragmentCustomFuncStr);
        const usize fragmentFuncPassthroughPos = fragmentCppStr.find(fragmentFuncPassthroughCall);
        if (fragmentFuncPassthroughPos != std::string::npos)
            fragmentCppStr.replace(fragmentFuncPassthroughPos, fragmentFuncPassthroughCall.length(), "");
    }

    std::vector<SShaderDefine> definesVec = {};
    for (usize i = 0; i < defineCount; i++)
        definesVec.push_back(defines[i]);
    DefineInShader(vertexCppStr, fragmentCppStr, definesVec);

    std::string vertexCppStrInclude = "";
    std::string fragmentCppStrInclude = "";
    CFileSystem* fs = _context->GetSubsystem<CFileSystem>();
    cMemoryAllocator* ma = _context->GetMemoryAllocator();
    if (vertexIncludePathCount > 0)
    {
        usize vertexIncludeBufferByteCount = 0;
        for (usize i = 0; i < vertexIncludePathCount; i++)
            vertexIncludeBufferByteCount += fs->TellFileByteSize(vertexIncludePaths[i]);
        u8* vertexIncludeBuffer = (u8*)ma->Allocate(vertexIncludeBufferByteCount, 64);
        usize vertexIncludeBufferPtr = 0;
        for (usize i = 0; i < vertexIncludePathCount; i++)
            vertexIncludeBufferPtr += fs->BinFileToArray(vertexIncludePaths[i], &vertexIncludeBuffer[0], vertexIncludeBufferPtr, vertexIncludeBufferByteCount);
        vertexCppStrInclude = std::string((const char*)&vertexIncludeBuffer[0], vertexIncludeBufferByteCount);
        ma->Deallocate(vertexIncludeBuffer);
    }
    if (fragmentIncludePathCount > 0)
    {
        usize fragmentIncludeBufferByteCount = 0;
        for (usize i = 0; i < fragmentIncludePathCount; i++)
            fragmentIncludeBufferByteCount += fs->TellFileByteSize(fragmentIncludePaths[i]);
        u8* fragmentIncludeBuffer = (u8*)ma->Allocate(fragmentIncludeBufferByteCount, 64);
        usize fragmentIncludeBufferPtr = 0;
        for (usize i = 0; i < fragmentIncludePathCount; i++)
            fragmentIncludeBufferPtr += fs->BinFileToArray(fragmentIncludePaths[i], &fragmentIncludeBuffer[0], fragmentIncludeBufferPtr, fragmentIncludeBufferByteCount);
        fragmentCppStrInclude = std::string((const char*)&fragmentIncludeBuffer[0], fragmentIncludeBufferByteCount);
        ma->Deallocate(fragmentIncludeBuffer);
    }

    const std::string finalVertexCppStr = CleanShaderSource(appendStr + vertexCppStrInclude + "\n" + vertexCppStr);
    const std::string finalFragmentCppStr = CleanShaderSource(appendStr + fragmentCppStrInclude + "\n" + fragmentCppStr);

    const char* vertexShaderStrPtr = finalVertexCppStr.c_str();
    const char* fragmentShaderStrPtr = finalFragmentCppStr.c_str();
    const GLint vertexShaderStrByteSize = strlen(vertexShaderStrPtr);
    const GLint fragmentShaderStrByteSize = strlen(fragmentShaderStrPtr);
    GLuint instance = glCreateProgram();
    const GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderStrPtr, &vertexShaderStrByteSize);
    glShaderSource(fragmentShader, 1, &fragmentShaderStrPtr, &fragmentShaderStrByteSize);
    glCompileShader(vertexShader);
    glCompileShader(fragmentShader);
    glAttachShader(instance, vertexShader);
    glAttachShader(instance, fragmentShader);
    glLinkProgram(instance);
    GLint success;
    glGetProgramiv(instance, GL_LINK_STATUS, &success);
    if (!success)
        Print("Error: can't link shader!");
    if (!glIsProgram(instance))
        Print("Error: invalid shader!");
    GLint logBufferByteSize = 0;
    GLchar logBuffer[1024] = {};
    glGetShaderInfoLog(vertexShader, 1024, &logBufferByteSize, &logBuffer[0]);
    if (logBufferByteSize > 0)
    {
        Print("Error: can't compile vertex shader!");
        Print(logBuffer);
    }
    logBufferByteSize = 0;
    glGetShaderInfoLog(fragmentShader, 1024, &logBufferByteSize, &logBuffer[0]);
    if (logBufferByteSize > 0)
    {
        Print("Error: can't compile fragment shader!");
        Print(logBuffer);
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return CGPUShaderResource(instance, 0);
}

void triton::XGraphicsBackendOGL::BindShader(const CGPUShaderResource& shader)
{
    const GLuint shaderID = (GLuint)shader.GetInstance();
    glUseProgram(shaderID);
}

void triton::XGraphicsBackendOGL::UnbindShader()
{
    glUseProgram(0);
}

void triton::XGraphicsBackendOGL::DestroyShader(const CGPUShaderResource& shader)
{
    glDeleteProgram(shader.GetInstance());
}

void triton::XGraphicsBackendOGL::SetShaderUniform(
    const CGPUShaderResource& shader,
    const std::string& name,
    const glm::mat4& matrix
)
{
    glUniformMatrix4fv(glGetUniformLocation(shader.GetInstance(), name.c_str()), 1, GL_FALSE, &matrix[0][0]);
}

void triton::XGraphicsBackendOGL::SetShaderUniform(
    const CGPUShaderResource& shader,
    const std::string& name,
    types::usize count,
    const types::f32* values
)
{
    glUniform4fv(glGetUniformLocation(shader.GetInstance(), name.c_str()), count, &values[0]);
}

void triton::XGraphicsBackendOGL::SetShaderUniform(
    const CGPUShaderResource& shader,
    const std::string& name,
    types::u32 value
)
{
    glUniform1ui(glGetUniformLocation(shader.GetInstance(), name.c_str()), value);
}

void triton::XGraphicsBackendOGL::SetShaderUniform(
    const CGPUShaderResource& shader,
    const std::string& name,
    types::s32 value
)
{
    glUniform1i(glGetUniformLocation(shader.GetInstance(), name.c_str()), value);
}

void triton::XGraphicsBackendOGL::BindTextureNamed(
    const CGPUShaderResource& shader,
    const CGPUTextureResource& texture,
    const std::string& textureName,
    types::s32 slot
)
{
    if (slot == -1)
        slot = texture.GetSlot();

    if (texture.GetDimension() == ETextureDimension::Texture2D)
    {
        glUniform1i(glGetUniformLocation((GLuint)shader.GetInstance(), textureName.c_str()), slot);
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, (GLuint)texture.GetInstance());
        glActiveTexture(GL_TEXTURE0);
    }
    else if (texture.GetDimension() == ETextureDimension::Texture2DArray)
    {
        glUniform1i(glGetUniformLocation((GLuint)shader.GetInstance(), textureName.c_str()), slot);
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D_ARRAY, (GLuint)texture.GetInstance());
        glActiveTexture(GL_TEXTURE0);
    }
}

triton::CGPUInputLayoutResource triton::XGraphicsBackendOGL::CreateInputLayout()
{
    GLuint instance = 0;
    glGenVertexArrays(1, (GLuint*)&instance);

    return CGPUInputLayoutResource(instance, 0);
}

void triton::XGraphicsBackendOGL::BindInputLayout(const CGPUInputLayoutResource& inputLayout)
{
    glBindVertexArray((GLuint)inputLayout.GetInstance());
}

void triton::XGraphicsBackendOGL::BindRigidInputLayout()
{
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 48, (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 48, (void*)12);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 48, (void*)20);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 48, (void*)32);
}

void triton::XGraphicsBackendOGL::BindSkinnedInputLayout()
{
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 80, (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 80, (void*)12);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 80, (void*)20);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 80, (void*)32);
    glVertexAttribIPointer(4, 4, GL_UNSIGNED_INT, 80, (void*)48);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 80, (void*)64);
}

void triton::XGraphicsBackendOGL::UnbindInputLayout()
{
    glBindVertexArray(0);
}

void triton::XGraphicsBackendOGL::DestroyInputLayout(const CGPUInputLayoutResource& vertexArray)
{
    GLuint instance = vertexArray.GetInstance();
    glDeleteVertexArrays(1, (GLuint*)&instance);
}

triton::CGPURenderPassResource triton::XGraphicsBackendOGL::CreateRenderPass()
{
    return CGPURenderPassResource(0, 0);
}

void triton::XGraphicsBackendOGL::BindRenderPass(const CGPURenderPassResource& renderPass)
{
}

void triton::XGraphicsBackendOGL::UnbindRenderPass(const CGPURenderPassResource& renderPass)
{
}

void triton::XGraphicsBackendOGL::BindDepthState(const SDepthState& blendMode)
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

void triton::XGraphicsBackendOGL::BindBlendState(const SBlendState& blendMode)
{
    for (usize i = 0; i < blendMode.factorCount; i++)
    {
        GLenum srcFactor = GL_ZERO;
        GLenum dstFactor = GL_ZERO;

        switch (blendMode.srcFactors[i])
        {
            case EBlendFactor::ONE: srcFactor = GL_ONE; break;
            case EBlendFactor::SRC_COLOR: srcFactor = GL_SRC_COLOR; break;
            case EBlendFactor::INV_SRC_COLOR: srcFactor = GL_ONE_MINUS_SRC_COLOR; break;
            case EBlendFactor::SRC_ALPHA: srcFactor = GL_SRC_ALPHA; break;
            case EBlendFactor::INV_SRC_ALPHA: srcFactor = GL_ONE_MINUS_SRC_ALPHA; break;
        }

        switch (blendMode.dstFactors[i])
        {
            case EBlendFactor::ONE: dstFactor = GL_ONE; break;
            case EBlendFactor::SRC_COLOR: dstFactor = GL_SRC_COLOR; break;
            case EBlendFactor::INV_SRC_COLOR: dstFactor = GL_ONE_MINUS_SRC_COLOR; break;
            case EBlendFactor::SRC_ALPHA: dstFactor = GL_SRC_ALPHA; break;
            case EBlendFactor::INV_SRC_ALPHA: dstFactor = GL_ONE_MINUS_SRC_ALPHA; break;
        }

        glBlendFunci(i, srcFactor, dstFactor);
    }
}

void triton::XGraphicsBackendOGL::SetViewport(const SViewport& viewport)
{
    glViewport(viewport.rect.GetX(), viewport.rect.GetY(), viewport.rect.GetZ(), viewport.rect.GetW());
}

triton::CGPURenderTargetResource triton::XGraphicsBackendOGL::CreateRenderTarget(
    const std::vector<CGPUTextureResource>& colorAttachments,
    const CGPUTextureResource& depthAttachment
)
{
    GLuint instance = 0;

    GLenum buffs[16] = {};
    glGenFramebuffers(1, (GLuint*)&instance);
    glBindFramebuffer(GL_FRAMEBUFFER, instance);
    for (usize i = 0; i < colorAttachments.size(); i++)
    {
        buffs[i] = GL_COLOR_ATTACHMENT0 + i;
        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0 + i,
            GL_TEXTURE_2D,
            colorAttachments[i].GetInstance(),
            0
        );
    }
    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_DEPTH_STENCIL_ATTACHMENT,
        GL_TEXTURE_2D,
        depthAttachment.GetInstance(),
        0
    );
    glDrawBuffers(colorAttachments.size(), &buffs[0]);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (status != GL_FRAMEBUFFER_COMPLETE)
        Print("Error: incomplete framebuffer!");

    return CGPURenderTargetResource(instance, 0, colorAttachments.size(), colorAttachments.data(), depthAttachment);
}

void triton::XGraphicsBackendOGL::ResizeRenderTargetColors(
    CGPURenderTargetResource& renderTarget,
    const glm::vec2& size
)
{
    std::vector<CGPUTextureResource> newColorAttachments;
    for (usize i = 0; i < renderTarget.GetColorAttachmentCount(); i++)
    {
        auto& attachment = renderTarget.GetColorAttachments()[i];
        newColorAttachments.emplace_back(
            CreateTexture(
                cVector3(size.x, size.y, attachment.GetDepth()),
                attachment.GetDimension(),
                attachment.GetFormat(),
                nullptr,
                0
            )
        );
        DestroyTexture(attachment);
    }
    renderTarget.SetColorAttachments(newColorAttachments.size(), newColorAttachments.data());

    GLenum buffs[16] = {};
    glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)renderTarget.GetInstance());
    for (usize i = 0; i < renderTarget.GetColorAttachmentCount(); i++)
    {
        buffs[i] = GL_COLOR_ATTACHMENT0 + i;
        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0 + i,
            GL_TEXTURE_2D,
            (GLuint)renderTarget.GetColorAttachments()[i].GetInstance(),
            0
        );
    }
    glDrawBuffers(renderTarget.GetColorAttachmentCount(), &buffs[0]);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void triton::XGraphicsBackendOGL::ResizeRenderTargetDepth(
    CGPURenderTargetResource& renderTarget,
    const glm::vec2& size
)
{
    CGPUTextureResource newDepthAttachment = CreateTexture(
        cVector3(size.x, size.y, renderTarget.GetDepthAttachment().GetDepth()),
        renderTarget.GetDepthAttachment().GetDimension(),
        renderTarget.GetDepthAttachment().GetFormat(),
        nullptr,
        0
    );
    DestroyTexture(renderTarget.GetDepthAttachment());
    renderTarget.SetDepthAttachment(newDepthAttachment);

    GLenum buffs[16] = {};
    glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)renderTarget.GetInstance());
    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_DEPTH_STENCIL_ATTACHMENT,
        GL_TEXTURE_2D,
        renderTarget.GetDepthAttachment().GetInstance(),
        0
    );
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void triton::XGraphicsBackendOGL::UpdateRenderTargetBuffers(CGPURenderTargetResource& renderTarget)
{
    GLuint instance = 0;

    GLenum buffs[16] = {};
    glGenFramebuffers(1, &instance);
    glBindFramebuffer(GL_FRAMEBUFFER, instance);
    for (usize i = 0; i < renderTarget.GetColorAttachmentCount(); i++)
    {
        buffs[i] = GL_COLOR_ATTACHMENT0 + i;
        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0 + i,
            GL_TEXTURE_2D,
            (GLuint)renderTarget.GetColorAttachments()[i].GetInstance(),
            0
        );
    }
    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_DEPTH_STENCIL_ATTACHMENT,
        GL_TEXTURE_2D,
        (GLuint)renderTarget.GetDepthAttachment().GetInstance(),
        0
    );
    glDrawBuffers(renderTarget.GetColorAttachmentCount(), &buffs[0]);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void triton::XGraphicsBackendOGL::BindRenderTarget(const CGPURenderTargetResource& renderTarget)
{
    glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)renderTarget.GetInstance());
}

void triton::XGraphicsBackendOGL::UnbindRenderTarget()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void triton::XGraphicsBackendOGL::DestroyRenderTarget(const CGPURenderTargetResource& renderTarget)
{
    UnbindRenderTarget();

    GLuint instance = renderTarget.GetInstance();
    glDeleteFramebuffers(1, &instance);
}

triton::CGPUBufferResource triton::XGraphicsBackendOGL::CreateBuffer(
    EGPUBufferType type,
    const types::u8* data,
    types::usize byteSize,
    types::s32 slot
)
{
    GLuint instance = 0;

    glGenBuffers(1, (GLuint*)&instance);

    if (type == EGPUBufferType::Vertex)
    {
        glBindBuffer(GL_ARRAY_BUFFER, instance);
        glBufferData(GL_ARRAY_BUFFER, byteSize, data, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    else if (type == EGPUBufferType::Index)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, instance);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, byteSize, data, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    else if (type == EGPUBufferType::Uniform)
    {
        glBindBuffer(GL_UNIFORM_BUFFER, instance);
        glBufferData(GL_UNIFORM_BUFFER, byteSize, data, GL_STATIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }
    else if (type == EGPUBufferType::Storage)
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, instance);
        glBufferData(GL_SHADER_STORAGE_BUFFER, byteSize, data, GL_STATIC_DRAW);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    return CGPUBufferResource(instance, 0, type, byteSize, slot);
}

void triton::XGraphicsBackendOGL::BindBuffer(const CGPUBufferResource& buffer)
{
    if (buffer.GetBufferType() == EGPUBufferType::Vertex)
        glBindBuffer(GL_ARRAY_BUFFER, (GLuint)buffer.GetInstance());
    else if (buffer.GetBufferType() == EGPUBufferType::Index)
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (GLuint)buffer.GetInstance());
    else if (buffer.GetBufferType() == EGPUBufferType::Uniform)
        glBindBufferBase(GL_UNIFORM_BUFFER, buffer.GetSlot(), (GLuint)buffer.GetInstance());
    else if (buffer.GetBufferType() == EGPUBufferType::Storage)
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, buffer.GetSlot(), buffer.GetInstance());
}

void triton::XGraphicsBackendOGL::BindBufferNotVAO(const CGPUBufferResource& buffer)
{
    if (buffer.GetBufferType() == EGPUBufferType::Uniform)
        glBindBufferBase(GL_UNIFORM_BUFFER, buffer.GetSlot(), (GLuint)buffer.GetInstance());
    else if (buffer.GetBufferType() == EGPUBufferType::Storage)
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, buffer.GetSlot(), buffer.GetInstance());
}

void triton::XGraphicsBackendOGL::UnbindBuffer(const CGPUBufferResource& buffer)
{
    if (buffer.GetBufferType() == EGPUBufferType::Vertex)
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    else if (buffer.GetBufferType() == EGPUBufferType::Index)
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    else if (buffer.GetBufferType() == EGPUBufferType::Uniform)
        glBindBufferBase(GL_UNIFORM_BUFFER, buffer.GetSlot(), 0);
    else if (buffer.GetBufferType() == EGPUBufferType::Storage)
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, buffer.GetSlot(), 0);
}

void triton::XGraphicsBackendOGL::WriteBuffer(
    const CGPUBufferResource& buffer,
    types::usize offset,
    types::usize byteSize,
    const types::u8* data
)
{
    if (buffer.GetBufferType() == EGPUBufferType::Vertex)
    {
        glBindBuffer(GL_ARRAY_BUFFER, buffer.GetInstance());
        glBufferSubData(GL_ARRAY_BUFFER, offset, byteSize, data);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    else if (buffer.GetBufferType() == EGPUBufferType::Index)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.GetInstance());
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, byteSize, data);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    else if (buffer.GetBufferType() == EGPUBufferType::Uniform)
    {
        glBindBuffer(GL_UNIFORM_BUFFER, buffer.GetInstance());
        glBufferSubData(GL_UNIFORM_BUFFER, offset, byteSize, data);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }
    else if (buffer.GetBufferType() == EGPUBufferType::Storage)
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer.GetInstance());
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, byteSize, data);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }
}

void triton::XGraphicsBackendOGL::DestroyBuffer(const CGPUBufferResource& buffer)
{
    if (buffer.GetBufferType() == EGPUBufferType::Vertex)
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    else if (buffer.GetBufferType() == EGPUBufferType::Index)
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    else if (buffer.GetBufferType() == EGPUBufferType::Uniform)
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    else if (buffer.GetBufferType() == EGPUBufferType::Storage)
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    GLuint instance = buffer.GetInstance();
    glDeleteBuffers(1, &instance);
}

triton::CGPUTextureResource triton::XGraphicsBackendOGL::CreateTexture(
    const cVector3& size,
    ETextureDimension dimension,
    ETextureFormat format,
    const types::u8* data,
    types::s32 slot
)
{
    GLuint instance = 0;

    glGenTextures(1, (GLuint*)&instance);

    GLenum formatGL = GL_RGBA8;
    GLenum channelsGL = GL_RGBA;
    GLenum formatComponentGL = GL_UNSIGNED_BYTE;
    if (format == ETextureFormat::R8)
    {
        formatGL = GL_R8;
        channelsGL = GL_RED;
        formatComponentGL = GL_UNSIGNED_BYTE;
    }
    else if (format == ETextureFormat::R8F)
    {
        formatGL = GL_R8;
        channelsGL = GL_RED;
        formatComponentGL = GL_FLOAT;
    }
    else if (format == ETextureFormat::RGBA8)
    {
        formatGL = GL_RGBA8;
        channelsGL = GL_RGBA;
        formatComponentGL = GL_UNSIGNED_BYTE;
    }
    else if (format == ETextureFormat::RGBA8_SRGB_Mips)
    {
        formatGL = GL_SRGB8_ALPHA8;
        channelsGL = GL_RGBA;
        formatComponentGL = GL_UNSIGNED_BYTE;
    }
    else if (format == ETextureFormat::RGB16F)
    {
        formatGL = GL_RGB16F;
        channelsGL = GL_RGB;
        formatComponentGL = GL_HALF_FLOAT;
    }
    else if (format == ETextureFormat::RGBA16F)
    {
        formatGL = GL_RGBA16F;
        channelsGL = GL_RGBA;
        formatComponentGL = GL_HALF_FLOAT;
    }
    else if (format == ETextureFormat::DepthStencil)
    {
        formatGL = GL_DEPTH24_STENCIL8;
        channelsGL = GL_DEPTH_STENCIL;
        formatComponentGL = GL_UNSIGNED_INT_24_8;
    }

    if (dimension == ETextureDimension::Texture2D)
    {
        glBindTexture(GL_TEXTURE_2D, instance);

        glTexImage2D(GL_TEXTURE_2D, 0, formatGL, size.GetX(), size.GetY(), 0, channelsGL, formatComponentGL, data);
        if (format != ETextureFormat::DepthStencil)
        {
            if (format == ETextureFormat::RGBA8_SRGB_Mips)
            {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            }
            else
            {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            }
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }

        glBindTexture(GL_TEXTURE_2D, 0);
    }
    else if (dimension == ETextureDimension::Texture2DArray)
    {
        glBindTexture(GL_TEXTURE_2D_ARRAY, instance);

        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, formatGL, size.GetX(), size.GetY(), size.GetZ(), 0, channelsGL, formatComponentGL, data);

        if (format == ETextureFormat::RGBA8_SRGB_Mips)
        {
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
        else
        {
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    }

    return CGPUTextureResource(instance, 0, size, dimension, format, slot);
}

triton::CGPUTextureResource triton::XGraphicsBackendOGL::ResizeTexture(const CGPUTextureResource& texture, const cVector2& size)
{
    CGPUTextureResource newTexture = CreateTexture(
        cVector3(size.GetX(), size.GetY(), texture.GetDepth()),
        texture.GetDimension(),
        texture.GetFormat(),
        nullptr,
        texture.GetSlot()
    );
    DestroyTexture(texture);

    return newTexture;
}

void triton::XGraphicsBackendOGL::BindTexture(const CGPUTextureResource& texture)
{
    if (texture.GetDimension() == ETextureDimension::Texture2D)
    {
        glActiveTexture(GL_TEXTURE0 + texture.GetSlot());
        glBindTexture(GL_TEXTURE_2D, texture.GetInstance());
        glActiveTexture(GL_TEXTURE0);
    }
    else if (texture.GetDimension() == ETextureDimension::Texture2DArray)
    {
        glActiveTexture(GL_TEXTURE0 + texture.GetSlot());
        glBindTexture(GL_TEXTURE_2D_ARRAY, texture.GetInstance());
        glActiveTexture(GL_TEXTURE0);
    }
}

void triton::XGraphicsBackendOGL::UnbindTexture(const CGPUTextureResource& texture)
{
    if (texture.GetDimension() == ETextureDimension::Texture2D)
        glBindTexture(GL_TEXTURE_2D, 0);
    else if (texture.GetDimension() == ETextureDimension::Texture2DArray)
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void triton::XGraphicsBackendOGL::WriteTexture(
    const CGPUTextureResource& texture,
    const cVector3& offset,
    const cVector2& size,
    const types::u8* data
)
{
    GLenum formatGL = GL_RGBA8;
    GLenum channelsGL = GL_RGBA;
    GLenum formatComponentGL = GL_UNSIGNED_BYTE;

    if (texture.GetFormat() == ETextureFormat::R8)
    {
        formatGL = GL_R8;
        channelsGL = GL_RED;
        formatComponentGL = GL_UNSIGNED_BYTE;
    }
    else if (texture.GetFormat() == ETextureFormat::R8F)
    {
        formatGL = GL_R8;
        channelsGL = GL_RED;
        formatComponentGL = GL_FLOAT;
    }
    else if (texture.GetFormat() == ETextureFormat::RGBA8_SRGB_Mips)
    {
        formatGL = GL_SRGB8_ALPHA8;
        channelsGL = GL_RGBA;
        formatComponentGL = GL_UNSIGNED_BYTE;
    }
    else if (texture.GetFormat() == ETextureFormat::RGBA8)
    {
        formatGL = GL_RGBA8;
        channelsGL = GL_RGBA;
        formatComponentGL = GL_UNSIGNED_BYTE;
    }
    else if (texture.GetFormat() == ETextureFormat::RGB16F)
    {
        formatGL = GL_RGB16F;
        channelsGL = GL_RGB;
        formatComponentGL = GL_HALF_FLOAT;
    }
    else if (texture.GetFormat() == ETextureFormat::RGBA16F)
    {
        formatGL = GL_RGBA16F;
        channelsGL = GL_RGBA;
        formatComponentGL = GL_HALF_FLOAT;
    }
    else if (texture.GetFormat() == ETextureFormat::DepthStencil)
    {
        formatGL = GL_DEPTH24_STENCIL8;
        channelsGL = GL_DEPTH_STENCIL;
        formatComponentGL = GL_UNSIGNED_INT_24_8;
    }

    if (texture.GetDimension() == ETextureDimension::Texture2D)
    {
        glBindTexture(GL_TEXTURE_2D, texture.GetInstance());
        glTexSubImage2D(
            GL_TEXTURE_2D,
            0,
            offset.GetX(),
            offset.GetY(),
            size.GetX(),
            size.GetY(),
            channelsGL,
            formatComponentGL,
            data
        );
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    else if (texture.GetDimension() == ETextureDimension::Texture2DArray)
    {
        if (offset.GetX() + size.GetX() <= texture.GetWidth() &&
            offset.GetY() + size.GetY() <= texture.GetHeight() &&
            offset.GetZ() < texture.GetDepth())
        {
            glBindTexture(GL_TEXTURE_2D_ARRAY, texture.GetInstance());
            glTexSubImage3D(
                GL_TEXTURE_2D_ARRAY,
                0,
                offset.GetX(),
                offset.GetY(),
                offset.GetZ(),
                size.GetX(),
                size.GetY(),
                1,
                channelsGL,
                formatComponentGL,
                data
            );
            glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
        }
    }
}

void triton::XGraphicsBackendOGL::WriteTextureToFile(const CGPUTextureResource& texture, const std::string& filename)
{
    if (texture.GetFormat() != ETextureFormat::RGBA8)
        return;

    GLenum channelsGL = GL_RGBA;
    GLenum formatComponentGL = GL_UNSIGNED_BYTE;
    usize formatByteCount = 4;

    if (texture.GetFormat() == ETextureFormat::RGBA8)
    {
        channelsGL = GL_RGBA;
        formatComponentGL = GL_UNSIGNED_BYTE;
        formatByteCount = 4;
    }

    if (texture.GetDimension() == ETextureDimension::Texture2D)
    {
        const sCapabilities& caps = _context->GetSubsystem<CEngine>()->GetApplication()->GetCapabilities();
        cMemoryAllocator* memoryAllocator = _context->GetMemoryAllocator();

        u8* pixels = (u8*)memoryAllocator->Allocate(
            texture.GetWidth() * texture.GetHeight() * formatByteCount,
            caps.memoryAlignment
        );

        glBindTexture(GL_TEXTURE_2D, texture.GetInstance());
        glGetTexImage(GL_TEXTURE_2D, 0, channelsGL, formatComponentGL, pixels);
        glBindTexture(GL_TEXTURE_2D, 0);

        // TODO: Implement texture to file write

        memoryAllocator->Deallocate(pixels);
    }
}

void triton::XGraphicsBackendOGL::GenerateTextureMips(const CGPUTextureResource& texture)
{
    if (texture.GetDimension() == ETextureDimension::Texture2D)
    {
        glBindTexture(GL_TEXTURE_2D, texture.GetInstance());
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    else if (texture.GetDimension() == ETextureDimension::Texture2DArray)
    {
        glBindTexture(GL_TEXTURE_2D_ARRAY, texture.GetInstance());
        glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    }
}

void triton::XGraphicsBackendOGL::DestroyTexture(const CGPUTextureResource& texture)
{
    if (texture.GetDimension() == ETextureDimension::Texture2D)
        glBindTexture(GL_TEXTURE_2D, 0);
    else if (texture.GetDimension() == ETextureDimension::Texture2DArray)
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

    GLuint instance = texture.GetInstance();
    glDeleteTextures(1, (GLuint*)&instance);
}