// graphics_pipeline_backend_ogl.cpp

#include <GL/glew.h>
#include <lodepng.h> // TODO: move lodepng stuff to separate backend
#include "graphics_pipeline_backend_ogl.hpp"
#include "graphics_resource_backend.hpp"
#include "context.hpp"
#include "filesystem_manager.hpp"
#include "application.hpp"
#include "instance_buffer.hpp"

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

void triton::cGraphicsPipelineBackendOGL::BindShader(const cShader* shader)
{
    const GLuint shaderID = (GLuint)shader->GetInstance();
    glUseProgram(shaderID);
}

void triton::cGraphicsPipelineBackendOGL::UnbindShader()
{
    glUseProgram(0);
}

triton::CGPUShader triton::cGraphicsPipelineBackendOGL::CreateShader(
    EBuiltinRenderPassType builtinType,
    const std::string& vertexStr,
    const std::string& fragmentStr,
    const std::string& vertexCustomFuncStr,
    const std::string& fragmentCustomFuncStr,
    const std::vector<SShaderDefine>& defines = {}
)
{
    std::string finalVertexStr = vertexStr;
    std::string finalFragmentStr = fragmentStr;

    std::string header = "";
    switch (builtinType)
    {
    case EBuiltinRenderPassType::NONE:
        Print("Error: invalid builtin render pass type 'RENDER_PATH_NONE' when creating a shader!");
        return CGPUShader(_context, 0, 0);

    case EBuiltinRenderPassType::OPAQUE_PATH:
        header = "RENDER_PATH_OPAQUE";
        break;

    case EBuiltinRenderPassType::TRANSPARENT_PATH:
        header = "RENDER_PATH_TRANSPARENT";
        break;

    case EBuiltinRenderPassType::TEXT_PATH:
        header = "RENDER_PATH_TEXT";
        break;

    case EBuiltinRenderPassType::TRANSPARENT_COMPOSITE_PATH:
        header = "RENDER_PATH_TRANSPARENT_COMPOSITE";
        break;

    case EBuiltinRenderPassType::QUAD_PATH:
        header = "RENDER_PATH_QUAD";
        break;
    }

    const std::string appendStr = "#version 430\n\n#define " + header + "\n\n";

    const std::string vertexFuncDefinition = "void Vertex_Func(in vec3 _positionLocal, in vec2 _texcoord, in vec3 _normal, in int _instanceID, in Instance _instance, in Material material, in float _use2D, out vec4 _glPosition){}";
    const std::string vertexFuncPassthroughCall = "Vertex_Passthrough(InPositionLocal, instance, instance.Use2D, gl_Position);";
    const std::string fragmentFuncDefinition = "void Fragment_Func(in vec2 _texcoord, in vec4 _textureColor, in vec4 _materialDiffuseColor, out vec4 _fragColor){}";
    const std::string fragmentFuncPassthroughCall = "Fragment_Passthrough(textureColor, DiffuseColor, fragColor);";

    if (!vertexCustomFuncStr.empty() && !fragmentCustomFuncStr.empty())
    {
        const usize vertexFuncDefinitionPos = finalVertexStr.find(vertexFuncDefinition);
        if (vertexFuncDefinitionPos != std::string::npos)
            finalVertexStr.replace(vertexFuncDefinitionPos, vertexFuncDefinition.length(), vertexCustomFuncStr);
        const usize vertexFuncPasstroughCallPos = finalVertexStr.find(vertexFuncPassthroughCall);
        if (vertexFuncPasstroughCallPos != std::string::npos)
            finalVertexStr.replace(vertexFuncPasstroughCallPos, vertexFuncPassthroughCall.length(), "");
        const usize fragmentFuncDefinitionPos = finalFragmentStr.find(fragmentFuncDefinition);
        if (fragmentFuncDefinitionPos != std::string::npos)
            finalFragmentStr.replace(fragmentFuncDefinitionPos, fragmentFuncDefinition.length(), fragmentCustomFuncStr);
        const usize fragmentFuncPassthroughPos = finalFragmentStr.find(fragmentFuncPassthroughCall);
        if (fragmentFuncPassthroughPos != std::string::npos)
            finalFragmentStr.replace(fragmentFuncPassthroughPos, fragmentFuncPassthroughCall.length(), "");
    }

    finalVertexStr = CleanShaderSource(finalVertexStr);
    finalFragmentStr = CleanShaderSource(finalFragmentStr);

    DefineInShader(finalVertexStr, finalFragmentStr, defines);

    finalVertexStr = appendStr + finalVertexStr;
    finalFragmentStr = appendStr + finalFragmentStr;

    const char* vertexShaderStrPtr = finalVertexStr.c_str();
    const char* fragmentShaderStrPtr = finalFragmentStr.c_str();
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
        Print("Error: vertex shader, header: " + header + "!");
        Print(logBuffer);
    }
    logBufferByteSize = 0;
    glGetShaderInfoLog(fragmentShader, 1024, &logBufferByteSize, &logBuffer[0]);
    if (logBufferByteSize > 0)
    {
        Print("Error: fragment shader, header: " + header + "!");
        Print(logBuffer);
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return CGPUShader(_context, instance, 0);
}

triton::cShader* triton::cGraphicsPipelineBackendOGL::CreateShader(
    const cShader* baseShader,
    const std::string& vertexFunc,
    const std::string& fragmentFunc,
    const std::vector<cShader::sDefinePair>& definePairs
)
{
    // TODO: rewrite shader creation logic
    // This must be an interface ---> CreateShader(vertexStr, fragmentStr, vertexFunc, fragmentFunc, defines={})
    const std::string vertexFuncDefinition = "void Vertex_Func(in vec3 _positionLocal, in vec2 _texcoord, in vec3 _normal, in int _instanceID, in Instance _instance, in Material material, in float _use2D, out vec4 _glPosition){}";
    const std::string vertexFuncPassthroughCall = "Vertex_Passthrough(InPositionLocal, instance, instance.Use2D, gl_Position);";
    const std::string fragmentFuncDefinition = "void Fragment_Func(in vec2 _texcoord, in vec4 _textureColor, in vec4 _materialDiffuseColor, out vec4 _fragColor){}";
    const std::string fragmentFuncPassthroughCall = "Fragment_Passthrough(textureColor, DiffuseColor, fragColor);";

    std::string vertexShaderStr = baseShader->GetVertexStr();
    std::string fragmentShaderStr = baseShader->GetFragmentStr();

    const usize vertexFuncDefinitionPos = vertexShaderStr.find(vertexFuncDefinition);
    if (vertexFuncDefinitionPos != std::string::npos)
        vertexShaderStr.replace(vertexFuncDefinitionPos, vertexFuncDefinition.length(), vertexFunc);
    const usize vertexFuncPasstroughCallPos = vertexShaderStr.find(vertexFuncPassthroughCall);
    if (vertexFuncPasstroughCallPos != std::string::npos)
        vertexShaderStr.replace(vertexFuncPasstroughCallPos, vertexFuncPassthroughCall.length(), "");

    const usize fragmentFuncDefinitionPos = fragmentShaderStr.find(fragmentFuncDefinition);
    if (fragmentFuncDefinitionPos != std::string::npos)
        fragmentShaderStr.replace(fragmentFuncDefinitionPos, fragmentFuncDefinition.length(), fragmentFunc);
    const usize fragmentFuncPassthroughPos = fragmentShaderStr.find(fragmentFuncPassthroughCall);
    if (fragmentFuncPassthroughPos != std::string::npos)
        fragmentShaderStr.replace(fragmentFuncPassthroughPos, fragmentFuncPassthroughCall.length(), "");

    vertexShaderStr = CleanShaderSource(vertexShaderStr);
    fragmentShaderStr = CleanShaderSource(fragmentShaderStr);

    DefineInShader(vertexShaderStr, fragmentShaderStr, definePairs);

    const usize vertexVersionPos = vertexShaderStr.find("#version 430");
    if (vertexVersionPos != std::string::npos)
        vertexShaderStr.replace(vertexVersionPos, std::string("#version 430").length(), "");
    const usize fragmentVersionPos = fragmentShaderStr.find("#version 430");
    if (fragmentVersionPos != std::string::npos)
        fragmentShaderStr.replace(fragmentVersionPos, std::string("#version 430").length(), "");

    vertexShaderStr = "#version 430\n\n" + vertexShaderStr;
    fragmentShaderStr = "#version 430\n\n" + fragmentShaderStr;

    const char* vertex = vertexShaderStr.c_str();
    const char* fragment = fragmentShaderStr.c_str();
    const GLint vertexByteSize = strlen(vertex);
    const GLint fragmentByteSize = strlen(fragment);
    GLuint instance = glCreateProgram();
    const GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(vertexShader, 1, &vertex, &vertexByteSize);
    glShaderSource(fragmentShader, 1, &fragment, &fragmentByteSize);
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
        Print("Error: vertex shader!");
        Print(logBuffer);
    }
    logBufferByteSize = 0;
    glGetShaderInfoLog(fragmentShader, 1024, &logBufferByteSize, &logBuffer[0]);
    if (logBufferByteSize > 0)
    {
        Print("Error: fragment shader!");
        Print(logBuffer);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    cShader* shader = _context->Create<cShader>(_context, instance, vertexShaderStr, fragmentShaderStr);

    return shader;
}

void triton::cGraphicsPipelineBackendOGL::DestroyShader(const CGPUShader& shader)
{
    glDeleteProgram(shader.GetInstance());
}

void triton::cGraphicsPipelineBackendOGL::SetShaderUniform(const cShader* shader, const std::string& name, const glm::mat4& matrix)
{
    glUniformMatrix4fv(glGetUniformLocation(shader->GetInstance(), name.c_str()), 1, GL_FALSE, &matrix[0][0]);
}

void triton::cGraphicsPipelineBackendOGL::SetShaderUniform(const cShader* shader, const std::string& name, usize count, const f32* values)
{
    glUniform4fv(glGetUniformLocation(shader->GetInstance(), name.c_str()), count, &values[0]);
}

void triton::cGraphicsPipelineBackendOGL::BindTextureNamed(
    cShader* shader,
    cTexture* texture,
    const std::string& textureName,
    types::u32 slot
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

triton::cVertexArray* triton::cGraphicsPipelineBackendOGL::CreateVertexArray()
{
    GLuint instance = 0;

    glGenVertexArrays(1, (GLuint*)&instance);

    cVertexArray* vertexArray = _context->Create<cVertexArray>(_context, instance);

    return vertexArray;
}

void triton::cGraphicsPipelineBackendOGL::BindVertexArray(const cVertexArray* vertexArray)
{
    glBindVertexArray((GLuint)vertexArray->GetInstance());
}

void triton::cGraphicsPipelineBackendOGL::BindDefaultVertexArray(const std::vector<cBuffer*>& buffersToBind)
{
    iGraphicsResourceBackend* resourceBackend = _context->GetBackend<iGraphicsResourceBackend>();

    static cVertexArray* vertexArray = nullptr;

    if (vertexArray == nullptr)
    {
        vertexArray = CreateVertexArray();

        BindVertexArray(vertexArray);
        for (auto buffer : buffersToBind)
            resourceBackend->BindBuffer(buffer);
        BindDefaultInputLayout();
        UnbindVertexArray();
    }

    BindVertexArray(vertexArray);
}

void triton::cGraphicsPipelineBackendOGL::UnbindVertexArray()
{
    glBindVertexArray(0);
}

void triton::cGraphicsPipelineBackendOGL::DestroyVertexArray(cVertexArray* vertexArray)
{
    GLuint instance = vertexArray->GetInstance();
    glDeleteVertexArrays(1, (GLuint*)&instance);

    if (vertexArray != nullptr)
        _context->Destroy<cVertexArray>(vertexArray);
}

triton::CGPURenderPass triton::cGraphicsPipelineBackendOGL::CreateRenderPass()
{
    return CGPURenderPass(_context, 0, 0);
}

void triton::cGraphicsPipelineBackendOGL::BindRenderPass(const XRenderPass* renderPass, cShader* customShader)
{
    iGraphicsResourceBackend* resourceBackend = _context->GetBackend<iGraphicsResourceBackend>();
    iGraphicsPipelineBackend* pipelineBackend = _context->GetBackend<iGraphicsPipelineBackend>();

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
        resourceBackend->BindBufferNotVAO(buffer);
    BindDepthMode(renderPass->GetDepthMode());
    BindBlendMode(renderPass->GetBlendMode());
    for (usize i = 0; i < renderPass->GetInputTextures().size(); i++)
        pipelineBackend->BindTextureNamed(
            shader,
            renderPass->GetInputTextures()[i],
            renderPass->GetInputTextureNames()[i],
            i
        );
}

void triton::cGraphicsPipelineBackendOGL::UnbindRenderPass(const XRenderPass* renderPass)
{
    iGraphicsResourceBackend* resourceBackend = _context->GetBackend<iGraphicsResourceBackend>();

    UnbindVertexArray();
    if (renderPass->GetRenderTarget() != nullptr)
        UnbindRenderTarget();
    for (auto buffer : renderPass->GetInputBuffers())
        resourceBackend->UnbindBuffer(buffer);
    for (auto texture : renderPass->GetInputTextures())
        resourceBackend->UnbindTexture(texture);
}

void triton::cGraphicsPipelineBackendOGL::DestroyRenderPass(XRenderPassGPU* renderPass)
{
    glBindVertexArray(0);
    DestroyVertexArray(renderPass->GetVertexArray());

    DestroyShader(renderPass->GetShader());

    _context->Destroy<XRenderPassGPU>(renderPass);
}

void triton::cGraphicsPipelineBackendOGL::BindDefaultInputLayout()
{
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 32, (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 32, (void*)12);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 32, (void*)20);
}

void triton::cGraphicsPipelineBackendOGL::BindDepthMode(const sDepthMode& blendMode)
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

void triton::cGraphicsPipelineBackendOGL::BindBlendMode(const sBlendMode& blendMode)
{
    for (usize i = 0; i < blendMode.factorCount; i++)
    {
        GLuint srcFactor = GL_ZERO;
        GLuint dstFactor = GL_ZERO;

        switch (blendMode.srcFactors[i])
        {
            case sBlendMode::eBlendFactor::ONE: srcFactor = GL_ONE; break;
            case sBlendMode::eBlendFactor::SRC_COLOR: srcFactor = GL_SRC_COLOR; break;
            case sBlendMode::eBlendFactor::INV_SRC_COLOR: srcFactor = GL_ONE_MINUS_SRC_COLOR; break;
            case sBlendMode::eBlendFactor::SRC_ALPHA: srcFactor = GL_SRC_ALPHA; break;
            case sBlendMode::eBlendFactor::INV_SRC_ALPHA: srcFactor = GL_ONE_MINUS_SRC_ALPHA; break;
        }

        switch (blendMode.dstFactors[i])
        {
            case sBlendMode::eBlendFactor::ONE: dstFactor = GL_ONE; break;
            case sBlendMode::eBlendFactor::SRC_COLOR: dstFactor = GL_SRC_COLOR; break;
            case sBlendMode::eBlendFactor::INV_SRC_COLOR: dstFactor = GL_ONE_MINUS_SRC_COLOR; break;
            case sBlendMode::eBlendFactor::SRC_ALPHA: dstFactor = GL_SRC_ALPHA; break;
            case sBlendMode::eBlendFactor::INV_SRC_ALPHA: dstFactor = GL_ONE_MINUS_SRC_ALPHA; break;
        }

        glBlendFunci(i, srcFactor, dstFactor);
    }
}

void triton::cGraphicsPipelineBackendOGL::Viewport(const sViewport& viewport)
{
    glViewport(viewport.rect.GetX(), viewport.rect.GetY(), viewport.rect.GetZ(), viewport.rect.GetW());
}

triton::cRenderTarget* triton::cGraphicsPipelineBackendOGL::CreateRenderTarget(
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

    cRenderTarget* renderTarget = _context->Create<cRenderTarget>(
        _context,
        instance,
        colorAttachments,
        depthAttachment
    );

    return renderTarget;
}

void triton::cGraphicsPipelineBackendOGL::ResizeRenderTargetColors(
    cRenderTarget* renderTarget,
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
    cRenderTarget* renderTarget,
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

void triton::cGraphicsPipelineBackendOGL::UpdateRenderTargetBuffers(cRenderTarget*& renderTarget)
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
    _context->Destroy<cRenderTarget>(renderTarget);
    renderTarget = _context->Create<cRenderTarget>(_context, instance, colorAttachments, depthAttachments);
}

void triton::cGraphicsPipelineBackendOGL::BindRenderTarget(const cRenderTarget* renderTarget)
{
    glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)renderTarget->GetInstance());
}

void triton::cGraphicsPipelineBackendOGL::UnbindRenderTarget()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void triton::cGraphicsPipelineBackendOGL::DestroyRenderTarget(cRenderTarget* renderTarget)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    GLuint instance = renderTarget->GetInstance();
    glDeleteFramebuffers(1, &instance);

    if (renderTarget != nullptr)
        _context->Destroy<cRenderTarget>(renderTarget);
}