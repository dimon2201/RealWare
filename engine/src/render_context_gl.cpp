// render_context_gl.cpp

#include <iostream>
#include <cstring>
#include <string>
#include <lodepng.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "buffer.hpp"
#include "render_context.hpp"
#include "filesystem_manager.hpp"
#include "types.hpp"
#include "application.hpp"
#include "render_manager.hpp"
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

triton::cOGLGraphicsBackend::cOGLGraphicsBackend(cContext* context) : iGraphicsBackend(context) {}

triton::cOGLGraphicsBackend::~cOGLGraphicsBackend() {}

void triton::cOGLGraphicsBackend::BindContext(void* nativeWindow)
{
    glfwMakeContextCurrent((GLFWwindow*)nativeWindow);
    glfwSwapInterval(1);

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

triton::cBuffer* triton::cOGLGraphicsBackend::CreateBuffer(usize byteSize, cBuffer::eType type, s32 slot, const void* data)
{
    cBuffer* buffer = _context->Create<cBuffer>(_context);
    buffer->_byteSize = byteSize;
    buffer->_type = type;
    buffer->_slot = slot;

    glGenBuffers(1, (GLuint*)&buffer->_instance);

    if (buffer->GetBufferType() == cBuffer::eType::VERTEX)
    {
        glBindBuffer(GL_ARRAY_BUFFER, (GLuint)buffer->_instance);
        glBufferData(GL_ARRAY_BUFFER, byteSize, data, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    else if (buffer->GetBufferType() == cBuffer::eType::INDEX)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (GLuint)buffer->_instance);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, byteSize, data, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    else if (buffer->GetBufferType() == cBuffer::eType::UNIFORM)
    {
        glBindBuffer(GL_UNIFORM_BUFFER, (GLuint)buffer->_instance);
        glBufferData(GL_UNIFORM_BUFFER, byteSize, data, GL_STATIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }
    else if (buffer->GetBufferType() == cBuffer::eType::LARGE)
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, (GLuint)buffer->_instance);
        glBufferData(GL_SHADER_STORAGE_BUFFER, byteSize, data, GL_STATIC_DRAW);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    return buffer;
}

void triton::cOGLGraphicsBackend::BindBuffer(const cBuffer* buffer)
{
    if (buffer->_type == cBuffer::eType::VERTEX)
        glBindBuffer(GL_ARRAY_BUFFER, (GLuint)buffer->_instance);
    else if (buffer->_type == cBuffer::eType::INDEX)
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (GLuint)buffer->_instance);
    else if (buffer->_type == cBuffer::eType::UNIFORM)
        glBindBufferBase(GL_UNIFORM_BUFFER, buffer->_slot, (GLuint)buffer->_instance);
    else if (buffer->_type == cBuffer::eType::LARGE)
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, buffer->_slot, buffer->_instance);
}
		
void triton::cOGLGraphicsBackend::BindBufferNotVAO(const cBuffer* buffer)
{
    if (buffer->_type == cBuffer::eType::UNIFORM)
        glBindBufferBase(GL_UNIFORM_BUFFER, buffer->_slot, (GLuint)buffer->_instance);
    else if (buffer->_type == cBuffer::eType::LARGE)
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, buffer->_slot, buffer->_instance);
}

void triton::cOGLGraphicsBackend::UnbindBuffer(const cBuffer* buffer)
{
    if (buffer->_type == cBuffer::eType::VERTEX)
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    else if (buffer->_type == cBuffer::eType::INDEX)
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    else if (buffer->_type == cBuffer::eType::UNIFORM)
        glBindBufferBase(GL_UNIFORM_BUFFER, buffer->_slot, 0);
    else if (buffer->_type == cBuffer::eType::LARGE)
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, buffer->_slot, 0);
}

void triton::cOGLGraphicsBackend::WriteBuffer(const cBuffer* buffer, usize offset, usize byteSize, const void* data)
{
    if (buffer->_type == cBuffer::eType::VERTEX)
    {
        glBindBuffer(GL_ARRAY_BUFFER, buffer->_instance);
        glBufferSubData(GL_ARRAY_BUFFER, offset, byteSize, data);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    else if (buffer->_type == cBuffer::eType::INDEX)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->_instance);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, byteSize, data);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    else if (buffer->_type == cBuffer::eType::UNIFORM)
    {
        glBindBuffer(GL_UNIFORM_BUFFER, buffer->_instance);
        glBufferSubData(GL_UNIFORM_BUFFER, offset, byteSize, data);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }
    else if (buffer->_type == cBuffer::eType::LARGE)
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer->_instance);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, byteSize, data);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }
}

void triton::cOGLGraphicsBackend::DestroyBuffer(cBuffer* buffer)
{
    if (buffer->_type == cBuffer::eType::VERTEX)
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    else if (buffer->_type == cBuffer::eType::INDEX)
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    else if (buffer->_type == cBuffer::eType::UNIFORM)
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    else if (buffer->_type == cBuffer::eType::LARGE)
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glDeleteBuffers(1, (GLuint*)&buffer->_instance);

    if (buffer != nullptr)
        _context->Destroy<cBuffer>(buffer);
}

triton::cVertexArray* triton::cOGLGraphicsBackend::CreateVertexArray()
{
    cVertexArray* vertexArray = _context->Create<cVertexArray>(_context);

    glGenVertexArrays(1, (GLuint*)&vertexArray->_instance);

    return vertexArray;
}

void triton::cOGLGraphicsBackend::BindVertexArray(const cVertexArray* vertexArray)
{
    glBindVertexArray((GLuint)vertexArray->_instance);
}

void triton::cOGLGraphicsBackend::BindDefaultVertexArray(const std::vector<cBuffer*>& buffersToBind)
{
    static cVertexArray* vertexArray = nullptr;

    if (vertexArray == nullptr)
    {
        vertexArray = CreateVertexArray();

        BindVertexArray(vertexArray);
        for (auto buffer : buffersToBind)
            BindBuffer(buffer);
        BindDefaultInputLayout();
        UnbindVertexArray();
    }

    BindVertexArray(vertexArray);
}

void triton::cOGLGraphicsBackend::UnbindVertexArray()
{
    glBindVertexArray(0);
}

void triton::cOGLGraphicsBackend::DestroyVertexArray(cVertexArray* vertexArray)
{
    glDeleteVertexArrays(1, (GLuint*)&vertexArray->_instance);

    if (vertexArray != nullptr)
        _context->Destroy<cVertexArray>(vertexArray);
}

void triton::cOGLGraphicsBackend::BindShader(const cShader* shader)
{
    const GLuint shaderID = (GLuint)shader->_instance;
    glUseProgram(shaderID);
}

void triton::cOGLGraphicsBackend::UnbindShader()
{
    glUseProgram(0);
}

triton::cShader* triton::cOGLGraphicsBackend::CreateShader(eCategory renderPath, const std::string& vertexPath, const std::string& fragmentPath, const std::vector<cShader::sDefinePair>& definePairs)
{
    cShader* shader = _context->Create<cShader>(_context);

    std::string header = "";
    switch (renderPath)
    {
        case eCategory::RENDER_PATH_NONE:
            Print("Error: invalid 'RENDER_PATH_NONE' for shaders '" + vertexPath + "' and '" + fragmentPath + "'!");
            return nullptr;

        case eCategory::RENDER_PATH_OPAQUE:
            header = "RENDER_PATH_OPAQUE";
            break;

        case eCategory::RENDER_PATH_TRANSPARENT:
            header = "RENDER_PATH_TRANSPARENT";
            break;

        case eCategory::RENDER_PATH_TEXT:
            header = "RENDER_PATH_TEXT";
            break;

        case eCategory::RENDER_PATH_TRANSPARENT_COMPOSITE:
            header = "RENDER_PATH_TRANSPARENT_COMPOSITE";
            break;

        case eCategory::RENDER_PATH_QUAD:
            header = "RENDER_PATH_QUAD";
            break;
    }

    const std::string appendStr = "#version 430\n\n#define " + header + "\n\n";

    cFileSystem* fileSystem = _context->GetSubsystem<cFileSystem>();
    cDataFile* vertexShaderFile = fileSystem->CreateDataFile(vertexPath, K_TRUE);
    shader->_vertex = CleanShaderSource(std::string((const char*)vertexShaderFile->GetBuffer()->GetData()));
    cDataFile* fragmentShaderFile = fileSystem->CreateDataFile(fragmentPath, K_TRUE);
    shader->_fragment = CleanShaderSource(std::string((const char*)fragmentShaderFile->GetBuffer()->GetData()));
            
    DefineInShader(shader, definePairs);

    shader->_vertex = appendStr + shader->_vertex;
    shader->_fragment = appendStr + shader->_fragment;

    const char* vertex = shader->_vertex.c_str();
    const char* fragment = shader->_fragment.c_str();

    const GLint vertexByteSize = strlen(vertex);
    const GLint fragmentByteSize = strlen(fragment);
    shader->_instance = glCreateProgram();
    const GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(vertexShader, 1, &vertex, &vertexByteSize);
    glShaderSource(fragmentShader, 1, &fragment, &fragmentByteSize);
    glCompileShader(vertexShader);
    glCompileShader(fragmentShader);
    glAttachShader(shader->_instance, vertexShader);
    glAttachShader(shader->_instance, fragmentShader);
    glLinkProgram(shader->_instance);
			
    GLint success;
    glGetProgramiv((GLuint)shader->_instance, GL_LINK_STATUS, &success);
    if (!success)
        Print("Error: can't link shader!");
    if (!glIsProgram((GLuint)shader->_instance))
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

    return shader;
}

triton::cShader* triton::cOGLGraphicsBackend::CreateShader(const cShader* baseShader, const std::string& vertexFunc, const std::string& fragmentFunc, const std::vector<cShader::sDefinePair>& definePairs)
{
    cShader* shader = _context->Create<cShader>(_context);

    const std::string vertexFuncDefinition = "void Vertex_Func(in vec3 _positionLocal, in vec2 _texcoord, in vec3 _normal, in int _instanceID, in Instance _instance, in Material material, in float _use2D, out vec4 _glPosition){}";
    const std::string vertexFuncPassthroughCall = "Vertex_Passthrough(InPositionLocal, instance, instance.Use2D, gl_Position);";
    const std::string fragmentFuncDefinition = "void Fragment_Func(in vec2 _texcoord, in vec4 _textureColor, in vec4 _materialDiffuseColor, out vec4 _fragColor){}";
    const std::string fragmentFuncPassthroughCall = "Fragment_Passthrough(textureColor, DiffuseColor, fragColor);";

    shader->_vertex = baseShader->_vertex;
    shader->_fragment = baseShader->_fragment;

    const usize vertexFuncDefinitionPos = shader->_vertex.find(vertexFuncDefinition);
    if (vertexFuncDefinitionPos != std::string::npos)
        shader->_vertex.replace(vertexFuncDefinitionPos, vertexFuncDefinition.length(), vertexFunc);
    const usize vertexFuncPasstroughCallPos = shader->_vertex.find(vertexFuncPassthroughCall);
    if (vertexFuncPasstroughCallPos != std::string::npos)
        shader->_vertex.replace(vertexFuncPasstroughCallPos, vertexFuncPassthroughCall.length(), "");

    const usize fragmentFuncDefinitionPos = shader->_fragment.find(fragmentFuncDefinition);
    if (fragmentFuncDefinitionPos != std::string::npos)
        shader->_fragment.replace(fragmentFuncDefinitionPos, fragmentFuncDefinition.length(), fragmentFunc);
    const usize fragmentFuncPassthroughPos = shader->_fragment.find(fragmentFuncPassthroughCall);
    if (fragmentFuncPassthroughPos != std::string::npos)
        shader->_fragment.replace(fragmentFuncPassthroughPos, fragmentFuncPassthroughCall.length(), "");

    shader->_vertex = CleanShaderSource(shader->_vertex);
    shader->_fragment = CleanShaderSource(shader->_fragment);

    DefineInShader(shader, definePairs);

    const usize vertexVersionPos = shader->_vertex.find("#version 430");
    if (vertexVersionPos != std::string::npos)
        shader->_vertex.replace(vertexVersionPos, std::string("#version 430").length(), "");
    const usize fragmentVersionPos = shader->_fragment.find("#version 430");
    if (fragmentVersionPos != std::string::npos)
        shader->_fragment.replace(fragmentVersionPos, std::string("#version 430").length(), "");

    shader->_vertex = "#version 430\n\n" + shader->_vertex;
    shader->_fragment = "#version 430\n\n" + shader->_fragment;

    const char* vertex = shader->_vertex.c_str();
    const char* fragment = shader->_fragment.c_str();
    const GLint vertexByteSize = strlen(vertex);
    const GLint fragmentByteSize = strlen(fragment);
    shader->_instance = glCreateProgram();
    const GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(vertexShader, 1, &vertex, &vertexByteSize);
    glShaderSource(fragmentShader, 1, &fragment, &fragmentByteSize);
    glCompileShader(vertexShader);
    glCompileShader(fragmentShader);
    glAttachShader(shader->_instance, vertexShader);
    glAttachShader(shader->_instance, fragmentShader);
    glLinkProgram(shader->_instance);

    GLint success;
    glGetProgramiv((GLuint)shader->_instance, GL_LINK_STATUS, &success);
    if (!success)
        Print("Error: can't link shader!");
    if (!glIsProgram((GLuint)shader->_instance))
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

    return shader;
}

void triton::cOGLGraphicsBackend::DefineInShader(cShader* shader, const std::vector<cShader::sDefinePair>& definePairs)
{
    if (!definePairs.empty())
    {
        std::string defineStr = "";
        for (const auto& define : definePairs)
            defineStr += "#define " + define._name + " " + std::to_string(define._index) + "\n";

        shader->_vertex = defineStr + shader->_vertex;
        shader->_fragment = defineStr + shader->_fragment;
    }
}

void triton::cOGLGraphicsBackend::DestroyShader(cShader* shader)
{
    glDeleteProgram(shader->_instance);

    if (shader != nullptr)
        _context->Destroy<cShader>(shader);
}

void triton::cOGLGraphicsBackend::SetShaderUniform(const cShader* shader, const std::string& name, const glm::mat4& matrix)
{
    glUniformMatrix4fv(glGetUniformLocation(shader->_instance, name.c_str()), 1, GL_FALSE, &matrix[0][0]);
}

void triton::cOGLGraphicsBackend::SetShaderUniform(const cShader* shader, const std::string& name, usize count, const f32* values)
{
    glUniform4fv(glGetUniformLocation(shader->_instance, name.c_str()), count, &values[0]);
}

triton::cTexture* triton::cOGLGraphicsBackend::CreateTexture(usize width, usize height, usize depth, cTexture::eDimension dimension, cTexture::eFormat format, const void* data)
{
    cTexture* texture = _context->Create<cTexture>(_context);
    texture->_width = width;
    texture->_height = height;
    texture->_depth = depth;
    texture->_dimension = dimension;
    texture->_format = format;

    glGenTextures(1, (GLuint*)&texture->_instance);

    GLenum formatGL = GL_RGBA8;
    GLenum channelsGL = GL_RGBA;
    GLenum formatComponentGL = GL_UNSIGNED_BYTE;
    if (texture->_format == cTexture::eFormat::R8)
    {
        formatGL = GL_R8;
        channelsGL = GL_RED;
        formatComponentGL = GL_UNSIGNED_BYTE;
    }
    else if (texture->_format == cTexture::eFormat::R8F)
    {
        formatGL = GL_R8;
        channelsGL = GL_RED;
        formatComponentGL = GL_FLOAT;
    }
    else if (texture->_format == cTexture::eFormat::RGBA8 || texture->_format == cTexture::eFormat::RGBA8_MIPS)
    {
        formatGL = GL_RGBA8;
        channelsGL = GL_RGBA;
        formatComponentGL = GL_UNSIGNED_BYTE;
    }
    else if (texture->_format == cTexture::eFormat::RGB16F)
    {
        formatGL = GL_RGB16F;
        channelsGL = GL_RGB;
        formatComponentGL = GL_HALF_FLOAT;
    }
    else if (texture->_format == cTexture::eFormat::RGBA16F)
    {
        formatGL = GL_RGBA16F;
        channelsGL = GL_RGBA;
        formatComponentGL = GL_HALF_FLOAT;
    }
    else if (texture->_format == cTexture::eFormat::DEPTH_STENCIL)
    {
        formatGL = GL_DEPTH24_STENCIL8;
        channelsGL = GL_DEPTH_STENCIL;
        formatComponentGL = GL_UNSIGNED_INT_24_8;
    }

    if (texture->GetDimension() == cTexture::eDimension::TEXTURE_2D)
    {
        glBindTexture(GL_TEXTURE_2D, texture->_instance);

        glTexImage2D(GL_TEXTURE_2D, 0, formatGL, texture->_width, texture->_height, 0, channelsGL, formatComponentGL, data);
        if (texture->_format != cTexture::eFormat::DEPTH_STENCIL)
        {
            if (texture->_format == cTexture::eFormat::RGBA8_MIPS)
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
    else if (texture->GetDimension() == cTexture::eDimension::TEXTURE_2D_ARRAY)
    {
        glBindTexture(GL_TEXTURE_2D_ARRAY, texture->_instance);

        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, formatGL, texture->_width, texture->_height, texture->_depth, 0, channelsGL, formatComponentGL, data);
                
        if (texture->_format == cTexture::eFormat::RGBA8_MIPS)
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

    return texture;
}

triton::cTexture* triton::cOGLGraphicsBackend::ResizeTexture(cTexture* texture, const glm::vec2& size)
{
    cTexture* newTexture = CreateTexture(size.x, size.y, texture->GetDepth(), texture->GetDimension(), texture->GetFormat(), nullptr);
    DestroyTexture(texture);

    return newTexture;
}

void triton::cOGLGraphicsBackend::BindTexture(const cShader* shader, const std::string& name, const cTexture* texture, s32 slot)
{
    if (slot == -1)
        slot = texture->_slot;

    if (texture->GetDimension() == cTexture::eDimension::TEXTURE_2D)
    {
        glUniform1i(glGetUniformLocation(shader->_instance, name.c_str()), slot);
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, texture->_instance);
        glActiveTexture(GL_TEXTURE0);
    }
    else if (texture->GetDimension() == cTexture::eDimension::TEXTURE_2D_ARRAY)
    {
        glUniform1i(glGetUniformLocation(shader->_instance, name.c_str()), slot);
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D_ARRAY, texture->_instance);
        glActiveTexture(GL_TEXTURE0);
    }
}

void triton::cOGLGraphicsBackend::UnbindTexture(const cTexture* texture)
{
    if (texture->GetDimension() == cTexture::eDimension::TEXTURE_2D_ARRAY)
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void triton::cOGLGraphicsBackend::WriteTexture(const cTexture* texture, const glm::vec3& offset, const glm::vec2& size, const void* data)
{
    GLenum formatGL = GL_RGBA8;
    GLenum channelsGL = GL_RGBA;
    GLenum formatComponentGL = GL_UNSIGNED_BYTE;

    if (texture->_format == cTexture::eFormat::R8)
    {
        formatGL = GL_R8;
        channelsGL = GL_RED;
        formatComponentGL = GL_UNSIGNED_BYTE;
    }
    else if (texture->_format == cTexture::eFormat::R8F)
    {
        formatGL = GL_R8;
        channelsGL = GL_RED;
        formatComponentGL = GL_FLOAT;
    }
    else if (texture->_format == cTexture::eFormat::RGBA8 || texture->_format == cTexture::eFormat::RGBA8_MIPS)
    {
        formatGL = GL_RGBA8;
        channelsGL = GL_RGBA;
        formatComponentGL = GL_UNSIGNED_BYTE;
    }
    else if (texture->_format == cTexture::eFormat::RGB16F)
    {
        formatGL = GL_RGB16F;
        channelsGL = GL_RGB;
        formatComponentGL = GL_HALF_FLOAT;
    }
    else if (texture->_format == cTexture::eFormat::RGBA16F)
    {
        formatGL = GL_RGBA16F;
        channelsGL = GL_RGBA;
        formatComponentGL = GL_HALF_FLOAT;
    }
    else if (texture->_format == cTexture::eFormat::DEPTH_STENCIL)
    {
        formatGL = GL_DEPTH24_STENCIL8;
        channelsGL = GL_DEPTH_STENCIL;
        formatComponentGL = GL_UNSIGNED_INT_24_8;
    }

    if (texture->GetDimension() == cTexture::eDimension::TEXTURE_2D)
    {
        glBindTexture(GL_TEXTURE_2D, texture->_instance);
        glTexSubImage2D(GL_TEXTURE_2D, 0, offset.x, offset.y, size.x, size.y, channelsGL, formatComponentGL, data);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    else if (texture->GetDimension() == cTexture::eDimension::TEXTURE_2D_ARRAY)
    {
        if (offset.x + size.x <= texture->_width && offset.y + size.y <= texture->_height && offset.z < texture->_depth)
        {
            glBindTexture(GL_TEXTURE_2D_ARRAY, texture->_instance);
            glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, offset.x, offset.y, offset.z, size.x, size.y, 1, channelsGL, formatComponentGL, data);
            glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
        }
    }
}

void triton::cOGLGraphicsBackend::WriteTextureToFile(const cTexture* texture, const std::string& filename)
{
    if (texture->GetFormat() != cTexture::eFormat::RGBA8)
        return;

    GLenum channelsGL = GL_RGBA;
    GLenum formatComponentGL = GL_UNSIGNED_BYTE;
    usize formatByteCount = 4;

    if (texture->GetFormat() == cTexture::eFormat::RGBA8)
    {
        channelsGL = GL_RGBA;
        formatComponentGL = GL_UNSIGNED_BYTE;
        formatByteCount = 4;
    }

    if (texture->GetDimension() == cTexture::eDimension::TEXTURE_2D)
    {
        const sCapabilities* caps = _context->GetSubsystem<cEngine>()->GetApplication()->GetCapabilities();
        cMemoryAllocator* memoryAllocator = _context->GetMemoryAllocator();

        u8* pixels = (u8*)memoryAllocator->Allocate(texture->GetWidth() * texture->GetHeight() * formatByteCount, caps->memoryAlignment);

        glBindTexture(GL_TEXTURE_2D, texture->_instance);
        glGetTexImage(GL_TEXTURE_2D, 0, channelsGL, formatComponentGL, pixels);
        glBindTexture(GL_TEXTURE_2D, 0);

        lodepng_encode32_file(filename.c_str(), pixels, texture->_width, texture->_height);

        memoryAllocator->Deallocate(pixels);
    }
}

void triton::cOGLGraphicsBackend::GenerateTextureMips(const cTexture* texture)
{
    if (texture->GetDimension() == cTexture::eDimension::TEXTURE_2D)
    {
        glBindTexture(GL_TEXTURE_2D, texture->_instance);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    else if (texture->GetDimension() == cTexture::eDimension::TEXTURE_2D_ARRAY)
    {
        glBindTexture(GL_TEXTURE_2D_ARRAY, texture->_instance);
        glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    }
}

void triton::cOGLGraphicsBackend::DestroyTexture(cTexture* texture)
{
    if (texture->GetDimension() == cTexture::eDimension::TEXTURE_2D)
        glBindTexture(GL_TEXTURE_2D, 0);
    else if (texture->GetDimension() == cTexture::eDimension::TEXTURE_2D_ARRAY)
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

    glDeleteTextures(1, (GLuint*)&texture->_instance);

    if (texture != nullptr)
        _context->Destroy<cTexture>(texture);
}

triton::cRenderTarget* triton::cOGLGraphicsBackend::CreateRenderTarget(const std::vector<cTexture*>& colorAttachments, cTexture* depthAttachment)
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

void triton::cOGLGraphicsBackend::ResizeRenderTargetColors(cRenderTarget* renderTarget, const glm::vec2& size)
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

void triton::cOGLGraphicsBackend::ResizeRenderTargetDepth(cRenderTarget* renderTarget, const glm::vec2& size)
{
    cTexture* newDepthAttachment = CreateTexture(size.x, size.y, renderTarget->_depthAttachment->GetDepth(), renderTarget->_depthAttachment->GetDimension(), renderTarget->_depthAttachment->GetFormat(), nullptr);
    DestroyTexture(renderTarget->_depthAttachment);
    renderTarget->_depthAttachment = newDepthAttachment;

    GLenum buffs[16] = {};
    glBindFramebuffer(GL_FRAMEBUFFER, renderTarget->_instance);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, renderTarget->_depthAttachment->_instance, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void triton::cOGLGraphicsBackend::UpdateRenderTargetBuffers(cRenderTarget* renderTarget)
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

void triton::cOGLGraphicsBackend::BindRenderTarget(const cRenderTarget* renderTarget)
{
    glBindFramebuffer(GL_FRAMEBUFFER, renderTarget->_instance);
}

void triton::cOGLGraphicsBackend::UnbindRenderTarget()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void triton::cOGLGraphicsBackend::DestroyRenderTarget(cRenderTarget* renderTarget)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, (GLuint*)&renderTarget->_instance);

    if (renderTarget != nullptr)
        _context->Destroy<cRenderTarget>(renderTarget);
}

triton::cRenderPassGPU* triton::cOGLGraphicsBackend::CreateRenderPass(const sRenderPassDescriptor& desc)
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
            BindBuffer(buffer);
    }
    else if (desc.inputVertexFormat == eCategory::VERTEX_BUFFER_FORMAT_POS_TEX_NRM_VEC3_VEC2_VEC3)
    {
        for (auto buffer : desc.inputBuffers)
            BindBuffer(buffer);

        BindDefaultInputLayout();
    }

    UnbindVertexArray();

    return _context->Create<cRenderPassGPU>(_context, vertexArray, shader);
}

void triton::cOGLGraphicsBackend::BindRenderPass(const cRenderPass* renderPass, cShader* customShader)
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
        BindBufferNotVAO(buffer);
    BindDepthMode(renderPass->GetDepthMode());
    BindBlendMode(renderPass->GetBlendMode());
    for (usize i = 0; i < renderPass->GetInputTextures().size(); i++)
        BindTexture(shader, renderPass->GetInputTextureNames()[i].c_str(), renderPass->GetInputTextures()[i], i);
}

void triton::cOGLGraphicsBackend::UnbindRenderPass(const cRenderPass* renderPass)
{
    UnbindVertexArray();
    if (renderPass->GetRenderTarget() != nullptr)
        UnbindRenderTarget();
    for (auto buffer : renderPass->GetInputBuffers())
        UnbindBuffer(buffer);
    for (auto texture : renderPass->GetInputTextures())
        UnbindTexture(texture);
}

void triton::cOGLGraphicsBackend::DestroyRenderPass(cRenderPassGPU* renderPass)
{
    glBindVertexArray(0);
    DestroyVertexArray(renderPass->GetVertexArray());
        
    DestroyShader(renderPass->GetShader());
        
    _context->Destroy<cRenderPassGPU>(renderPass);
}

void triton::cOGLGraphicsBackend::BindDefaultInputLayout()
{
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 32, (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 32, (void*)12);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 32, (void*)20);
}

void triton::cOGLGraphicsBackend::BindBlendMode(const sBlendMode& blendMode)
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

void triton::cOGLGraphicsBackend::BindDepthMode(const sDepthMode& blendMode)
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

void triton::cOGLGraphicsBackend::Viewport(const sViewport& viewport)
{
    glViewport(viewport.rect.GetX(), viewport.rect.GetY(), viewport.rect.GetZ(), viewport.rect.GetW());
}

void triton::cOGLGraphicsBackend::ClearColor(const glm::vec4& color)
{
    glClearColor(color.x, color.y, color.z, color.w);
    glClear(GL_COLOR_BUFFER_BIT);
}

void triton::cOGLGraphicsBackend::ClearDepth(const f32 depth)
{
    glClearDepth(depth);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void triton::cOGLGraphicsBackend::ClearFramebufferColor(usize bufferIndex, const glm::vec4& color)
{
    glClearBufferfv(GL_COLOR, bufferIndex, &color.x);
}

void triton::cOGLGraphicsBackend::ClearFramebufferDepth(f32 depth)
{
    glClearDepth(depth);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void triton::cOGLGraphicsBackend::Draw(usize indexCount, usize vertexOffset, usize indexOffset, usize instanceCount)
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

void triton::cOGLGraphicsBackend::DrawQuad()
{
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void triton::cOGLGraphicsBackend::DrawQuads(usize count)
{
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, count);
}