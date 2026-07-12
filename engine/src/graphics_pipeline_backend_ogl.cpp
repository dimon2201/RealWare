// graphics_pipeline_backend_ogl.cpp

#include <glbinding/gl/gl.h>
#include "graphics_pipeline_backend_ogl.hpp"
#include "graphics_resource_backend.hpp"
#include "context.hpp"
#include "filesystem_manager.hpp"
#include "application.hpp"
#include "instance_buffer.hpp"
#include "graphics.hpp"

using namespace gl;
using namespace types;

namespace triton
{
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

triton::cGraphicsPipelineBackendOGL::cGraphicsPipelineBackendOGL(cContext* context) : iGraphicsPipelineBackend(context) {}

void triton::cGraphicsPipelineBackendOGL::BindShader(const CGPUShader* shader)
{
    const GLuint shaderID = (GLuint)shader->GetInstance();
    glUseProgram(shaderID);
}

void triton::cGraphicsPipelineBackendOGL::UnbindShader()
{
    glUseProgram(0);
}

triton::CGPUShader triton::cGraphicsPipelineBackendOGL::CreateShader(
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
    cFileSystem* fs = _context->GetSubsystem<cFileSystem>();
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

    return CGPUShader(_context, instance, 0);
}

void triton::cGraphicsPipelineBackendOGL::DestroyShader(const CGPUShader& shader)
{
    glDeleteProgram(shader.GetInstance());
}

void triton::cGraphicsPipelineBackendOGL::SetShaderUniform(const CGPUShader* shader, const std::string& name, const glm::mat4& matrix)
{
    glUniformMatrix4fv(glGetUniformLocation(shader->GetInstance(), name.c_str()), 1, GL_FALSE, &matrix[0][0]);
}

void triton::cGraphicsPipelineBackendOGL::SetShaderUniform(const CGPUShader* shader, const std::string& name, usize count, const f32* values)
{
    glUniform4fv(glGetUniformLocation(shader->GetInstance(), name.c_str()), count, &values[0]);
}

void triton::cGraphicsPipelineBackendOGL::SetShaderUniform(const CGPUShader* shader, const std::string& name, types::u32 value)
{
    glUniform1ui(glGetUniformLocation(shader->GetInstance(), name.c_str()), value);
}

void triton::cGraphicsPipelineBackendOGL::SetShaderUniform(const CGPUShader* shader, const std::string& name, types::s32 value)
{
    glUniform1i(glGetUniformLocation(shader->GetInstance(), name.c_str()), value);
}

void triton::cGraphicsPipelineBackendOGL::BindTextureNamed(
    CGPUShader* shader,
    cTexture* texture,
    const std::string& textureName,
    types::s32 slot
)
{
    if (slot == -1)
        slot = texture->GetSlot();

    if (texture->GetDimension() == cTexture::eDimension::TEXTURE_2D)
    {
        glUniform1i(glGetUniformLocation((GLuint)shader->GetInstance(), textureName.c_str()), slot);
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, (GLuint)texture->GetInstance());
        glActiveTexture(GL_TEXTURE0);
    }
    else if (texture->GetDimension() == cTexture::eDimension::TEXTURE_2D_ARRAY)
    {
        glUniform1i(glGetUniformLocation((GLuint)shader->GetInstance(), textureName.c_str()), slot);
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D_ARRAY, (GLuint)texture->GetInstance());
        glActiveTexture(GL_TEXTURE0);
    }
}

triton::CGPUVertexArray triton::cGraphicsPipelineBackendOGL::CreateVertexArray()
{
    GLuint instance = 0;
    glGenVertexArrays(1, (GLuint*)&instance);

    return CGPUVertexArray(_context, instance);
}

void triton::cGraphicsPipelineBackendOGL::BindVertexArray(const CGPUVertexArray& vertexArray)
{
    glBindVertexArray((GLuint)vertexArray.GetInstance());
}

void triton::cGraphicsPipelineBackendOGL::BindDefaultVertexArray(const std::vector<cBuffer*>& buffersToBind)
{
    // TODO: do something with this method
    /*iGraphicsResourceBackend* resourceBackend = _context->GetBackend<iGraphicsResourceBackend>();

    static CVertexArray* vertexArray = nullptr;

    if (vertexArray == nullptr)
    {
        vertexArray = CreateVertexArray();

        BindVertexArray(vertexArray);
        for (auto buffer : buffersToBind)
            resourceBackend->BindBuffer(buffer);
        BindDefaultInputLayout();
        UnbindVertexArray();
    }

    BindVertexArray(vertexArray);*/
}

void triton::cGraphicsPipelineBackendOGL::UnbindVertexArray()
{
    glBindVertexArray(0);
}

void triton::cGraphicsPipelineBackendOGL::DestroyVertexArray(const CGPUVertexArray& vertexArray)
{
    GLuint instance = vertexArray.GetInstance();
    glDeleteVertexArrays(1, (GLuint*)&instance);
}

triton::CGPURenderPass triton::cGraphicsPipelineBackendOGL::CreateRenderPass()
{
    return CGPURenderPass(_context, 0, 0);
}

void triton::cGraphicsPipelineBackendOGL::BindRenderPass(const XRenderPass* renderPass, CGPUShader* customShader)
{
    // TODO: 99% that this method must be deleted
    // render pass binding must be done in XRenderPass
    /*iGraphicsResourceBackend* resourceBackend = _context->GetBackend<iGraphicsResourceBackend>();
    iGraphicsPipelineBackend* pipelineBackend = _context->GetBackend<iGraphicsPipelineBackend>();

    CGPUShader* shader = nullptr;
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
        resourceBackend->BindBufferNotVAO(buffer);
    BindDepthMode(renderPass->GetDepthMode());
    BindBlendMode(renderPass->GetBlendMode());
    for (usize i = 0; i < renderPass->GetInputTextures().size(); i++)
        pipelineBackend->BindTextureNamed(
            shader,
            renderPass->GetInputTextures()[i],
            renderPass->GetInputTextureNames()[i],
            i
        );*/
}

void triton::cGraphicsPipelineBackendOGL::UnbindRenderPass(const XRenderPass* renderPass)
{
    // TODO: 99% that this method must be deleted
    // render pass binding must be done in XRenderPass
    /*iGraphicsResourceBackend* resourceBackend = _context->GetBackend<iGraphicsResourceBackend>();

    UnbindVertexArray();
    if (renderPass->GetRenderTarget() != nullptr)
        UnbindRenderTarget();
    for (auto buffer : renderPass->GetInputBuffers())
        resourceBackend->UnbindBuffer(buffer);
    for (auto texture : renderPass->GetInputTextures())
        resourceBackend->UnbindTexture(texture);*/
}

void triton::cGraphicsPipelineBackendOGL::BindDefaultInputLayout()
{
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
    glEnableVertexAttribArray(5);
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 84, (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 84, (void*)12);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 84, (void*)20);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 84, (void*)32);
    glVertexAttribPointer(4, 1, GL_INT, GL_FALSE, 84, (void*)48);
    glVertexAttribPointer(5, 4, GL_UNSIGNED_INT, GL_FALSE, 84, (void*)52);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 84, (void*)68);
}

void triton::cGraphicsPipelineBackendOGL::BindDepthMode(const SDepthState& blendMode)
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

void triton::cGraphicsPipelineBackendOGL::BindBlendMode(const SBlendState& blendMode)
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

void triton::cGraphicsPipelineBackendOGL::Viewport(const SViewport& viewport)
{
    glViewport(viewport.rect.GetX(), viewport.rect.GetY(), viewport.rect.GetZ(), viewport.rect.GetW());
}

triton::XRenderTarget* triton::cGraphicsPipelineBackendOGL::CreateRenderTarget(
    const std::vector<cTexture*>& colorAttachments,
    cTexture* depthAttachment
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
            colorAttachments[i]->GetInstance(),
            0
        );
    }
    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_DEPTH_STENCIL_ATTACHMENT,
        GL_TEXTURE_2D,
        depthAttachment->GetInstance(),
        0
    );
    glDrawBuffers(colorAttachments.size(), &buffs[0]);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (status != GL_FRAMEBUFFER_COMPLETE)
        Print("Error: incomplete framebuffer!");

    XRenderTarget* renderTarget = _context->Create<XRenderTarget>(
        _context,
        instance,
        colorAttachments,
        depthAttachment
    );

    return renderTarget;
}

void triton::cGraphicsPipelineBackendOGL::ResizeRenderTargetColors(
    XRenderTarget* renderTarget,
    const glm::vec2& size
)
{
    iGraphicsResourceBackend* resourceBackend = _context->GetBackend<iGraphicsResourceBackend>();

    std::vector<cTexture*> newColorAttachments;
    for (auto attachment : renderTarget->_colorAttachments)
    {
        newColorAttachments.emplace_back(
            resourceBackend->CreateTexture(
                cVector3(size.x, size.y, attachment->GetDepth()),
                attachment->GetDimension(),
                attachment->GetFormat(),
                nullptr,
                0
            )
        );
        resourceBackend->DestroyTexture(attachment);
    }
    renderTarget->GetColorAttachments().clear();
    renderTarget->SetColorAttachments(newColorAttachments);

    GLenum buffs[16] = {};
    glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)renderTarget->GetInstance());
    for (usize i = 0; i < renderTarget->_colorAttachments.size(); i++)
    {
        buffs[i] = GL_COLOR_ATTACHMENT0 + i;
        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0 + i,
            GL_TEXTURE_2D,
            (GLuint)renderTarget->GetColorAttachments()[i]->GetInstance(),
            0
        );
    }
    glDrawBuffers(renderTarget->GetColorAttachments().size(), &buffs[0]);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void triton::cGraphicsPipelineBackendOGL::ResizeRenderTargetDepth(
    XRenderTarget* renderTarget,
    const glm::vec2& size
)
{
    iGraphicsResourceBackend* resourceBackend = _context->GetBackend<iGraphicsResourceBackend>();

    cTexture* newDepthAttachment = resourceBackend->CreateTexture(
        cVector3(size.x, size.y, renderTarget->GetDepthAttachment()->GetDepth()),
        renderTarget->GetDepthAttachment()->GetDimension(),
        renderTarget->GetDepthAttachment()->GetFormat(),
        nullptr,
        0
    );
    resourceBackend->DestroyTexture(renderTarget->GetDepthAttachment());
    renderTarget->SetDepthAttachment(newDepthAttachment);

    GLenum buffs[16] = {};
    glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)renderTarget->GetInstance());
    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_DEPTH_STENCIL_ATTACHMENT,
        GL_TEXTURE_2D,
        renderTarget->GetDepthAttachment()->GetInstance(),
        0
    );
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void triton::cGraphicsPipelineBackendOGL::UpdateRenderTargetBuffers(XRenderTarget*& renderTarget)
{
    GLuint instance = 0;

    GLenum buffs[16] = {};
    glGenFramebuffers(1, &instance);
    glBindFramebuffer(GL_FRAMEBUFFER, instance);
    for (usize i = 0; i < renderTarget->GetColorAttachments().size(); i++)
    {
        buffs[i] = GL_COLOR_ATTACHMENT0 + i;
        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0 + i,
            GL_TEXTURE_2D,
            (GLuint)renderTarget->GetColorAttachments()[i]->GetInstance(),
            0
        );
    }
    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_DEPTH_STENCIL_ATTACHMENT,
        GL_TEXTURE_2D,
        (GLuint)renderTarget->GetDepthAttachment()->GetInstance(),
        0
    );
    glDrawBuffers(renderTarget->GetColorAttachments().size(), &buffs[0]);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    const std::vector<cTexture*> colorAttachments = renderTarget->GetColorAttachments();
    cTexture* depthAttachments = renderTarget->GetDepthAttachment();
    _context->Destroy<XRenderTarget>(renderTarget);
    renderTarget = _context->Create<XRenderTarget>(_context, instance, colorAttachments, depthAttachments);
}

void triton::cGraphicsPipelineBackendOGL::BindRenderTarget(const XRenderTarget* renderTarget)
{
    glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)renderTarget->GetInstance());
}

void triton::cGraphicsPipelineBackendOGL::UnbindRenderTarget()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void triton::cGraphicsPipelineBackendOGL::DestroyRenderTarget(XRenderTarget* renderTarget)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    GLuint instance = renderTarget->GetInstance();
    glDeleteFramebuffers(1, &instance);

    if (renderTarget != nullptr)
        _context->Destroy<XRenderTarget>(renderTarget);
}