// graphics_pipeline_backend_ogl.cpp

#include <GL/glew.h>
#include <lodepng.h> // TODO: move lodepng stuff to separate backend
#include "graphics_pipeline_backend_ogl.hpp"
#include "graphics_texture_backend.hpp"
#include "context.hpp"
#include "filesystem_manager.hpp"

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

    void DefineInShader(std::string& shaderVertexStr, std::string& shaderFragmentStr, const std::vector<cShader::sDefinePair>& definePairs)
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

triton::cShader* triton::cGraphicsPipelineBackendOGL::CreateShader(
    cGraphics::eRenderPath renderPath,
    const std::string& vertexPath,
    const std::string& fragmentPath,
    const std::vector<cShader::sDefinePair>& definePairs
)
{
    std::string header = "";
    switch (renderPath)
    {
    case cGraphics::eRenderPath::NONE:
        Print("Error: invalid 'RENDER_PATH_NONE' for shaders '" + vertexPath + "' and '" + fragmentPath + "'!");
        return nullptr;

    case cGraphics::eRenderPath::OPAQUE_PATH:
        header = "RENDER_PATH_OPAQUE";
        break;

    case cGraphics::eRenderPath::TRANSPARENT_PATH:
        header = "RENDER_PATH_TRANSPARENT";
        break;

    case cGraphics::eRenderPath::TEXT_PATH:
        header = "RENDER_PATH_TEXT";
        break;

    case cGraphics::eRenderPath::TRANSPARENT_COMPOSITE_PATH:
        header = "RENDER_PATH_TRANSPARENT_COMPOSITE";
        break;

    case cGraphics::eRenderPath::QUAD_PATH:
        header = "RENDER_PATH_QUAD";
        break;
    }

    const std::string appendStr = "#version 430\n\n#define " + header + "\n\n";

    cFileSystem* fileSystem = _context->GetSubsystem<cFileSystem>();
    cDataFile* vertexShaderFile = fileSystem->CreateDataFile(vertexPath, K_TRUE);
    std::string vertexShaderStr = CleanShaderSource(std::string((const char*)vertexShaderFile->GetBuffer()->GetData()));
    cDataFile* fragmentShaderFile = fileSystem->CreateDataFile(fragmentPath, K_TRUE);
    std::string fragmentShaderStr = CleanShaderSource(std::string((const char*)fragmentShaderFile->GetBuffer()->GetData()));

    DefineInShader(vertexShaderStr, fragmentShaderStr, definePairs);

    vertexShaderStr = appendStr + vertexShaderStr;
    fragmentShaderStr = appendStr + fragmentShaderStr;

    const char* vertexShaderStrPtr = vertexShaderStr.c_str();
    const char* fragmentShaderStrPtr = fragmentShaderStr.c_str();

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
        Print("Error: vertex shader, header: " + header + ", path: " + vertexPath + "!");
        Print(logBuffer);
    }
    logBufferByteSize = 0;
    glGetShaderInfoLog(fragmentShader, 1024, &logBufferByteSize, &logBuffer[0]);
    if (logBufferByteSize > 0)
    {
        Print("Error: fragment shader, header: " + header + ", path: " + fragmentPath + "!");
        Print(logBuffer);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    fileSystem->DestroyDataFile(vertexShaderFile);
    fileSystem->DestroyDataFile(fragmentShaderFile);

    cShader* shader = _context->Create<cShader>(_context, instance, vertexShaderStr, fragmentShaderStr);

    return shader;
}

triton::cShader* triton::cGraphicsPipelineBackendOGL::CreateShader(
    const cShader* baseShader,
    const std::string& vertexFunc,
    const std::string& fragmentFunc,
    const std::vector<cShader::sDefinePair>& definePairs
)
{
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

void triton::cGraphicsPipelineBackendOGL::DestroyShader(cShader* shader)
{
    glDeleteProgram(shader->GetInstance());

    if (shader != nullptr)
        _context->Destroy<cShader>(shader);
}

void triton::cGraphicsPipelineBackendOGL::SetShaderUniform(const cShader* shader, const std::string& name, const glm::mat4& matrix)
{
    glUniformMatrix4fv(glGetUniformLocation(shader->GetInstance(), name.c_str()), 1, GL_FALSE, &matrix[0][0]);
}

void triton::cGraphicsPipelineBackendOGL::SetShaderUniform(const cShader* shader, const std::string& name, usize count, const f32* values)
{
    glUniform4fv(glGetUniformLocation(shader->GetInstance(), name.c_str()), count, &values[0]);
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
    iGraphicsBufferBackend* bufferBackend = _context->GetBackend<iGraphicsBufferBackend>();

    static cVertexArray* vertexArray = nullptr;

    if (vertexArray == nullptr)
    {
        vertexArray = CreateVertexArray();

        BindVertexArray(vertexArray);
        for (auto buffer : buffersToBind)
            bufferBackend->BindBuffer(buffer);
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

triton::cRenderPassGPU* triton::cGraphicsPipelineBackendOGL::CreateRenderPass(const sRenderPassDescriptor& desc)
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

    iGraphicsBufferBackend* bufferBackend = _context->GetBackend<iGraphicsBufferBackend>();

    vertexArray = CreateVertexArray();
    BindVertexArray(vertexArray);
    if (desc.inputVertexFormat == eCategory::VERTEX_BUFFER_FORMAT_NONE)
    {
        for (auto buffer : desc.inputBuffers)
            bufferBackend->BindBuffer(buffer);
    }
    else if (desc.inputVertexFormat == eCategory::VERTEX_BUFFER_FORMAT_POS_TEX_NRM_VEC3_VEC2_VEC3)
    {
        for (auto buffer : desc.inputBuffers)
            bufferBackend->BindBuffer(buffer);

        BindDefaultInputLayout();
    }

    UnbindVertexArray();

    return _context->Create<cRenderPassGPU>(_context, vertexArray, shader);
}

void triton::cGraphicsPipelineBackendOGL::BindRenderPass(const cRenderPass* renderPass, cShader* customShader)
{
    cShader* shader = nullptr;
    if (customShader == nullptr)
        shader = renderPass->GetRenderPassGPU()->GetShader();
    else
        shader = customShader;

    iGraphicsBufferBackend* bufferBackend = _context->GetBackend<iGraphicsBufferBackend>();
    iGraphicsTextureBackend* textureBackend = _context->GetBackend<iGraphicsTextureBackend>();

    BindShader(shader);
    BindVertexArray(renderPass->GetRenderPassGPU()->GetVertexArray());
    if (renderPass->GetRenderTarget() != nullptr)
        BindRenderTarget(renderPass->GetRenderTarget());
    else
        UnbindRenderTarget();
    Viewport(renderPass->GetViewport());
    for (auto buffer : renderPass->GetInputBuffers())
        bufferBackend->BindBufferNotVAO(buffer);
    BindDepthMode(renderPass->GetDepthMode());
    BindBlendMode(renderPass->GetBlendMode());
    // FIXME: find what to do with this function
    // |||||||||||||||||||||||||||||||||||||||||
    // |||||||||||||||||||||||||||||||||||||||||
    // VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
    //for (usize i = 0; i < renderPass->GetInputTextures().size(); i++)
    //    textureBackend->BindTexture(
    //        shader,
    //        renderPass->GetInputTextureNames()[i].c_str(),
    //        renderPass->GetInputTextures()[i],
    //        i
    //    );
}

void triton::cGraphicsPipelineBackendOGL::UnbindRenderPass(const cRenderPass* renderPass)
{
    iGraphicsBufferBackend* bufferBackend = _context->GetBackend<iGraphicsBufferBackend>();
    iGraphicsTextureBackend* textureBackend = _context->GetBackend<iGraphicsTextureBackend>();

    UnbindVertexArray();
    if (renderPass->GetRenderTarget() != nullptr)
        UnbindRenderTarget();
    for (auto buffer : renderPass->GetInputBuffers())
        bufferBackend->UnbindBuffer(buffer);
    for (auto texture : renderPass->GetInputTextures())
        textureBackend->UnbindTexture(texture);
}

void triton::cGraphicsPipelineBackendOGL::DestroyRenderPass(cRenderPassGPU* renderPass)
{
    glBindVertexArray(0);
    DestroyVertexArray(renderPass->GetVertexArray());

    DestroyShader(renderPass->GetShader());

    _context->Destroy<cRenderPassGPU>(renderPass);
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
            case cGraphics::eBlendFactor::ONE: srcFactor = GL_ONE; break;
            case cGraphics::eBlendFactor::SRC_COLOR: srcFactor = GL_SRC_COLOR; break;
            case cGraphics::eBlendFactor::INV_SRC_COLOR: srcFactor = GL_ONE_MINUS_SRC_COLOR; break;
            case cGraphics::eBlendFactor::SRC_ALPHA: srcFactor = GL_SRC_ALPHA; break;
            case cGraphics::eBlendFactor::INV_SRC_ALPHA: srcFactor = GL_ONE_MINUS_SRC_ALPHA; break;
        }

        switch (blendMode.dstFactors[i])
        {
            case cGraphics::eBlendFactor::ONE: dstFactor = GL_ONE; break;
            case cGraphics::eBlendFactor::SRC_COLOR: dstFactor = GL_SRC_COLOR; break;
            case cGraphics::eBlendFactor::INV_SRC_COLOR: dstFactor = GL_ONE_MINUS_SRC_COLOR; break;
            case cGraphics::eBlendFactor::SRC_ALPHA: dstFactor = GL_SRC_ALPHA; break;
            case cGraphics::eBlendFactor::INV_SRC_ALPHA: dstFactor = GL_ONE_MINUS_SRC_ALPHA; break;
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
    iGraphicsTextureBackend* textureBackend = _context->GetBackend<iGraphicsTextureBackend>();

    std::vector<cTexture*> newColorAttachments;
    for (auto attachment : renderTarget->_colorAttachments)
    {
        newColorAttachments.emplace_back(
            textureBackend->CreateTexture(
                cVector3(size.x, size.y, attachment->GetDepth()),
                attachment->GetDimension(),
                attachment->GetFormat(),
                nullptr,
                0
            )
        );
        textureBackend->DestroyTexture(attachment);
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
    iGraphicsTextureBackend* textureBackend = _context->GetBackend<iGraphicsTextureBackend>();

    cTexture* newDepthAttachment = textureBackend->CreateTexture(
        cVector3(size.x, size.y, renderTarget->GetDepthAttachment()->GetDepth()),
        renderTarget->GetDepthAttachment()->GetDimension(),
        renderTarget->GetDepthAttachment()->GetFormat(),
        nullptr,
        0
    );
    textureBackend->DestroyTexture(renderTarget->GetDepthAttachment());
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
    _context->Create<cRenderTarget>(_context, instance, colorAttachments, depthAttachments);
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