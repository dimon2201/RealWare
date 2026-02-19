// graphics_pipeline_backend_ogl.cpp

#include <GL/glew.h>
#include <lodepng.h> // TODO: move lodepng stuff to separate backend
#include "graphics_pipeline_backend_ogl.hpp"
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

triton::cGraphicsShaderBackendOGL::cGraphicsShaderBackendOGL(cContext* context) : iGraphicsShaderBackend(context) {}

void triton::cGraphicsShaderBackendOGL::BindShader(const cShader* shader)
{
    const GLuint shaderID = (GLuint)shader->GetInstance();
    glUseProgram(shaderID);
}

void triton::cGraphicsShaderBackendOGL::UnbindShader()
{
    glUseProgram(0);
}

triton::cShader* triton::cGraphicsShaderBackendOGL::CreateShader(
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

triton::cShader* triton::cGraphicsShaderBackendOGL::CreateShader(
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

void triton::cGraphicsShaderBackendOGL::DestroyShader(cShader* shader)
{
    glDeleteProgram(shader->GetInstance());

    if (shader != nullptr)
        _context->Destroy<cShader>(shader);
}

void triton::cGraphicsShaderBackendOGL::SetShaderUniform(const cShader* shader, const std::string& name, const glm::mat4& matrix)
{
    glUniformMatrix4fv(glGetUniformLocation(shader->GetInstance(), name.c_str()), 1, GL_FALSE, &matrix[0][0]);
}

void triton::cGraphicsShaderBackendOGL::SetShaderUniform(const cShader* shader, const std::string& name, usize count, const f32* values)
{
    glUniform4fv(glGetUniformLocation(shader->GetInstance(), name.c_str()), count, &values[0]);
}