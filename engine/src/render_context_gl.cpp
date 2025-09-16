#include <iostream>
#include <cstring>
#include <string>
#include <lodepng.h>
#include <GL/glew.h>
#include "render_context.hpp"
#include "filesystem_manager.hpp"
#include "types.hpp"
#include "application.hpp"
#include "render_manager.hpp"
#include "memory_pool.hpp"

using namespace types;

namespace realware
{
    using namespace app;
    using namespace game;
    using namespace utils;

    namespace render
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
            std::cout << message << std::endl;
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

        cOpenGLRenderContext::cOpenGLRenderContext(const cApplication* const app) : _app((cApplication*)app)
        {
            if (glewInit() != GLEW_OK)
            {
                std::cout << "Error initializing GLEW!" << std::endl;
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

        cOpenGLRenderContext::~cOpenGLRenderContext()
        {
        }

        sBuffer* cOpenGLRenderContext::CreateBuffer(const usize byteSize, const sBuffer::eType& type, const void* const data)
        {
            sBuffer* pBuffer = (sBuffer*)_app->GetMemoryPool()->Allocate(sizeof(sBuffer));
            sBuffer* buffer = new (pBuffer) sBuffer();

            buffer->ByteSize = byteSize;
            buffer->Type = type;
            buffer->Slot = 0;

            glGenBuffers(1, (GLuint*)&buffer->Instance);

            if (buffer->Type == sBuffer::eType::VERTEX)
            {
                glBindBuffer(GL_ARRAY_BUFFER, (GLuint)buffer->Instance);
                glBufferData(GL_ARRAY_BUFFER, byteSize, data, GL_STATIC_DRAW);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
            }
            else if (buffer->Type == sBuffer::eType::INDEX)
            {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (GLuint)buffer->Instance);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, byteSize, data, GL_STATIC_DRAW);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            }
            else if (buffer->Type == sBuffer::eType::UNIFORM)
            {
                glBindBuffer(GL_UNIFORM_BUFFER, (GLuint)buffer->Instance);
                glBufferData(GL_UNIFORM_BUFFER, byteSize, data, GL_STATIC_DRAW);
                glBindBuffer(GL_UNIFORM_BUFFER, 0);
            }
            else if (buffer->Type == sBuffer::eType::LARGE)
            {
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, (GLuint)buffer->Instance);
                glBufferData(GL_SHADER_STORAGE_BUFFER, byteSize, data, GL_STATIC_DRAW);
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
            }

            return buffer;
        }

        void cOpenGLRenderContext::BindBuffer(const sBuffer* const buffer)
        {
            if (buffer->Type == sBuffer::eType::VERTEX)
                glBindBuffer(GL_ARRAY_BUFFER, (GLuint)buffer->Instance);
            else if (buffer->Type == sBuffer::eType::INDEX)
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (GLuint)buffer->Instance);
            else if (buffer->Type == sBuffer::eType::UNIFORM)
                glBindBufferBase(GL_UNIFORM_BUFFER, buffer->Slot, (GLuint)buffer->Instance);
            else if (buffer->Type == sBuffer::eType::LARGE)
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, buffer->Slot, buffer->Instance);
        }
		
		void cOpenGLRenderContext::BindBufferNotVAO(const sBuffer* const buffer)
        {
            if (buffer->Type == sBuffer::eType::UNIFORM)
                glBindBufferBase(GL_UNIFORM_BUFFER, buffer->Slot, (GLuint)buffer->Instance);
            else if (buffer->Type == sBuffer::eType::LARGE)
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, buffer->Slot, buffer->Instance);
        }

        void cOpenGLRenderContext::UnbindBuffer(const sBuffer* const buffer)
        {
            if (buffer->Type == sBuffer::eType::VERTEX) {
                glBindBuffer(GL_ARRAY_BUFFER, 0);
            } else if (buffer->Type == sBuffer::eType::INDEX) {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            } else if (buffer->Type == sBuffer::eType::UNIFORM) {
                glBindBufferBase(GL_UNIFORM_BUFFER, buffer->Slot, 0);
            } else if (buffer->Type == sBuffer::eType::LARGE) {
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, buffer->Slot, 0);
            }
        }

        void cOpenGLRenderContext::WriteBuffer(const sBuffer* const buffer, const usize offset, const usize byteSize, const void* const data)
        {
            if (buffer->Type == sBuffer::eType::VERTEX)
            {
                glBindBuffer(GL_ARRAY_BUFFER, buffer->Instance);
                glBufferSubData(GL_ARRAY_BUFFER, offset, byteSize, data);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
            }
            else if (buffer->Type == sBuffer::eType::INDEX)
            {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->Instance);
                glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, byteSize, data);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            }
            else if (buffer->Type == sBuffer::eType::UNIFORM)
            {
                glBindBuffer(GL_UNIFORM_BUFFER, buffer->Instance);
                glBufferSubData(GL_UNIFORM_BUFFER, offset, byteSize, data);
                glBindBuffer(GL_UNIFORM_BUFFER, 0);
            }
            else if (buffer->Type == sBuffer::eType::LARGE)
            {
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer->Instance);
                glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, byteSize, data);
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
            }
        }

        void cOpenGLRenderContext::DestroyBuffer(sBuffer* buffer)
        {
            if (buffer->Type == sBuffer::eType::VERTEX)
                glBindBuffer(GL_ARRAY_BUFFER, 0);
            else if (buffer->Type == sBuffer::eType::INDEX)
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            else if (buffer->Type == sBuffer::eType::UNIFORM)
                glBindBuffer(GL_UNIFORM_BUFFER, 0);
            else if (buffer->Type == sBuffer::eType::LARGE)
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

            glDeleteBuffers(1, (GLuint*)&buffer->Instance);

            if (buffer != nullptr)
            {
                buffer->~sBuffer();
                _app->GetMemoryPool()->Free(buffer);
            }
        }

        sVertexArray* cOpenGLRenderContext::CreateVertexArray()
        {
            sVertexArray* pVertexArray = (sVertexArray*)_app->GetMemoryPool()->Allocate(sizeof(sVertexArray));
            sVertexArray* vertexArray = new (pVertexArray) sVertexArray();

            glGenVertexArrays(1, (GLuint*)&vertexArray->Instance);

            return vertexArray;
        }

        void cOpenGLRenderContext::BindVertexArray(const sVertexArray* const vertexArray)
        {
            glBindVertexArray((GLuint)vertexArray->Instance);
        }

        void cOpenGLRenderContext::BindDefaultVertexArray(const std::vector<sBuffer*>& buffersToBind)
        {
            static sVertexArray* vertexArray = nullptr;

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

        void cOpenGLRenderContext::UnbindVertexArray()
        {
            glBindVertexArray(0);
        }

        void cOpenGLRenderContext::DestroyVertexArray(sVertexArray* vertexArray)
        {
            glDeleteVertexArrays(1, (GLuint*)&vertexArray->Instance);

            if (vertexArray != nullptr)
            {
                vertexArray->~sVertexArray();
                _app->GetMemoryPool()->Free(vertexArray);
            }
        }

        void cOpenGLRenderContext::BindShader(const sShader* const shader)
        {
            auto shaderID = (GLuint)shader->Instance;
            glUseProgram(shaderID);
        }

        void cOpenGLRenderContext::UnbindShader()
        {
            glUseProgram(0);
        }

        sShader* cOpenGLRenderContext::CreateShader(const std::string& header, const std::string& vertexPath, const std::string& fragmentPath)
        {
            sShader* pShader = (sShader*)_app->GetMemoryPool()->Allocate(sizeof(sShader));
            sShader* shader = new (pShader) sShader();

            std::string appendStr = "#version 430\n\n#define " + header + "\n\n";
            std::string appendPathVertexStr = appendStr;
            std::string appendPathFragmentStr = appendStr;

            fs::sFile* vertexShaderFile = _app->GetFileSystemManager()->CreateDataFile(vertexPath, K_TRUE);
            shader->Vertex = CleanShaderSource(appendPathVertexStr.append(std::string((const char*)vertexShaderFile->Data)));
            const char* vertex = shader->Vertex.c_str();

            fs::sFile* fragmentShaderFile = _app->GetFileSystemManager()->CreateDataFile(fragmentPath, K_TRUE);
            shader->Fragment = CleanShaderSource(appendPathFragmentStr.append(std::string((const char*)fragmentShaderFile->Data)));
            const char* fragment = shader->Fragment.c_str();

            GLint vertexByteSize = strlen(vertex);
            GLint fragmentByteSize = strlen(fragment);

            shader->Instance = glCreateProgram();
            auto vertexShader = glCreateShader(GL_VERTEX_SHADER);
            auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(vertexShader, 1, &vertex, &vertexByteSize);
            glShaderSource(fragmentShader, 1, &fragment, &fragmentByteSize);
            glCompileShader(vertexShader);
            glCompileShader(fragmentShader);
            glAttachShader(shader->Instance, vertexShader);
            glAttachShader(shader->Instance, fragmentShader);
            glLinkProgram(shader->Instance);
			
            GLint success;
            glGetProgramiv((GLuint)shader->Instance, GL_LINK_STATUS, &success);
            if (!success)
                std::cout << "Error: Linking shader!" << std::endl;
            if (!glIsProgram((GLuint)shader->Instance))
                std::cout << "Error: Invalid shader!" << std::endl;

            GLint logBufferByteSize = 0;
            char logBuffer[1024] = {};
            glGetShaderInfoLog(vertexShader, 1024, &logBufferByteSize, &logBuffer[0]);
            if (logBufferByteSize > 0)
            {
                std::cout << "Vertex shader error, header: " << header << ", path: " << vertexPath << std::endl;
                std::cout << logBuffer << std::endl;
            }
            logBufferByteSize = 0;
            glGetShaderInfoLog(fragmentShader, 1024, &logBufferByteSize, &logBuffer[0]);
            if (logBufferByteSize > 0)
            {
                std::cout << "Fragment shader error, header: " << header << ", path: " << fragmentPath << std::endl;
                std::cout << logBuffer << std::endl;
            }
			
			glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);

            _app->GetFileSystemManager()->DestroyDataFile(vertexShaderFile);
            _app->GetFileSystemManager()->DestroyDataFile(fragmentShaderFile);

            return shader;
        }

        sShader* cOpenGLRenderContext::CreateShader(const sShader* const baseShader, const std::string& vertexFunc, const std::string& fragmentFunc)
        {
            sShader* pShader = (sShader*)_app->GetMemoryPool()->Allocate(sizeof(sShader));
            sShader* shader = new (pShader) sShader();

            const std::string vertexFuncDefinition = "void Vertex_Func(in vec3 _positionLocal, in vec2 _texcoord, in vec3 _normal, in int _instanceID, in Instance _instance, in Material material, in float _use2D, out vec4 _glPosition){}";
            const std::string vertexFuncPassthroughCall = "Vertex_Passthrough(InPositionLocal, instance, instance.Use2D, gl_Position);";
            const std::string fragmentFuncDefinition = "void Fragment_Func(in vec2 _texcoord, in vec4 _textureColor, in vec4 _materialDiffuseColor, out vec4 _fragColor){}";
            const std::string fragmentFuncPassthroughCall = "Fragment_Passthrough(textureColor, DiffuseColor, fragColor);";

            shader->Vertex = baseShader->Vertex;
            shader->Fragment = baseShader->Fragment;

            size_t vertexFuncDefinitionPos = shader->Vertex.find(vertexFuncDefinition);
            if (vertexFuncDefinitionPos != std::string::npos)
                shader->Vertex.replace(vertexFuncDefinitionPos, vertexFuncDefinition.length(), vertexFunc);
            size_t vertexFuncPasstroughCallPos = shader->Vertex.find(vertexFuncPassthroughCall);
            if (vertexFuncPasstroughCallPos != std::string::npos)
                shader->Vertex.replace(vertexFuncPasstroughCallPos, vertexFuncPassthroughCall.length(), "");

            size_t fragmentFuncDefinitionPos = shader->Fragment.find(fragmentFuncDefinition);
            if (fragmentFuncDefinitionPos != std::string::npos)
                shader->Fragment.replace(fragmentFuncDefinitionPos, fragmentFuncDefinition.length(), fragmentFunc);
            size_t fragmentFuncPassthroughPos = shader->Fragment.find(fragmentFuncPassthroughCall);
            if (fragmentFuncPassthroughPos != std::string::npos)
                shader->Fragment.replace(fragmentFuncPassthroughPos, fragmentFuncPassthroughCall.length(), "");

            shader->Vertex = CleanShaderSource(shader->Vertex);
            shader->Fragment = CleanShaderSource(shader->Fragment);

            const char* vertex = shader->Vertex.c_str();
            const char* fragment = shader->Fragment.c_str();

            GLint vertexByteSize = strlen(vertex);
            GLint fragmentByteSize = strlen(fragment);

            shader->Instance = glCreateProgram();
            auto vertexShader = glCreateShader(GL_VERTEX_SHADER);
            auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(vertexShader, 1, &vertex, &vertexByteSize);
            glShaderSource(fragmentShader, 1, &fragment, &fragmentByteSize);
            glCompileShader(vertexShader);
            glCompileShader(fragmentShader);
            glAttachShader(shader->Instance, vertexShader);
            glAttachShader(shader->Instance, fragmentShader);
            glLinkProgram(shader->Instance);

            GLint success;
            glGetProgramiv((GLuint)shader->Instance, GL_LINK_STATUS, &success);
            if (!success)
                std::cout << "Error: Linking shader!" << std::endl;
            if (!glIsProgram((GLuint)shader->Instance))
                std::cout << "Error: Invalid shader!" << std::endl;

            GLint logBufferByteSize = 0;
            char logBuffer[1024] = {};
            glGetShaderInfoLog(vertexShader, 1024, &logBufferByteSize, &logBuffer[0]);
            if (logBufferByteSize > 0)
            {
                std::cout << "Vertex shader error" << std::endl;
                std::cout << logBuffer << std::endl;
            }
            logBufferByteSize = 0;
            glGetShaderInfoLog(fragmentShader, 1024, &logBufferByteSize, &logBuffer[0]);
            if (logBufferByteSize > 0)
            {
                std::cout << "Fragment shader error" << std::endl;
                std::cout << logBuffer << std::endl;
            }

            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);

            return shader;
        }

        void cOpenGLRenderContext::DestroyShader(sShader* shader)
        {
            glDeleteProgram(shader->Instance);

            if (shader != nullptr)
            {
                shader->~sShader();
                _app->GetMemoryPool()->Free(shader);
            }
        }

        void cOpenGLRenderContext::SetShaderUniform(const sShader* const shader, const std::string& name, const glm::mat4& matrix)
        {
            glUniformMatrix4fv(glGetUniformLocation(shader->Instance, name.c_str()), 1, GL_FALSE, &matrix[0][0]);
        }

        void cOpenGLRenderContext::SetShaderUniform(const sShader* const shader, const std::string& name, const usize count, const f32* const values)
        {
            glUniform4fv(glGetUniformLocation(shader->Instance, name.c_str()), count, &values[0]);
        }

        sTexture* cOpenGLRenderContext::CreateTexture(const usize width, const usize height, const usize depth, const sTexture::eType& type, const sTexture::eFormat& format, const void* const data)
        {
            sTexture* pTexture = (sTexture*)_app->GetMemoryPool()->Allocate(sizeof(sTexture));
            sTexture* texture = new (pTexture) sTexture();
            
            texture->Width = width;
            texture->Height = height;
            texture->Depth = depth;
            texture->Type = type;
            texture->Format = format;

            glGenTextures(1, (GLuint*)&texture->Instance);

            GLenum formatGL = GL_RGBA8;
            GLenum channelsGL = GL_RGBA;
            GLenum formatComponentGL = GL_UNSIGNED_BYTE;
            if (texture->Format == sTexture::eFormat::R8)
            {
                formatGL = GL_R8;
                channelsGL = GL_RED;
                formatComponentGL = GL_UNSIGNED_BYTE;
            }
            else if (texture->Format == sTexture::eFormat::R8F)
            {
                formatGL = GL_R8;
                channelsGL = GL_RED;
                formatComponentGL = GL_FLOAT;
            }
            else if (texture->Format == sTexture::eFormat::RGBA8 ||
                texture->Format == sTexture::eFormat::RGBA8_MIPS)
            {
                formatGL = GL_RGBA8;
                channelsGL = GL_RGBA;
                formatComponentGL = GL_UNSIGNED_BYTE;
            }
            else if (texture->Format == sTexture::eFormat::RGB16F)
            {
                formatGL = GL_RGB16F;
                channelsGL = GL_RGB;
                formatComponentGL = GL_HALF_FLOAT;
            }
            else if (texture->Format == sTexture::eFormat::RGBA16F)
            {
                formatGL = GL_RGBA16F;
                channelsGL = GL_RGBA;
                formatComponentGL = GL_HALF_FLOAT;
            }
            else if (texture->Format == sTexture::eFormat::DEPTH_STENCIL)
            {
                formatGL = GL_DEPTH24_STENCIL8;
                channelsGL = GL_DEPTH_STENCIL;
                formatComponentGL = GL_UNSIGNED_INT_24_8;
            }

            if (texture->Type == sTexture::eType::TEXTURE_2D)
            {
                glBindTexture(GL_TEXTURE_2D, texture->Instance);
                glTexImage2D(GL_TEXTURE_2D, 0, formatGL, texture->Width, texture->Height, 0, channelsGL, formatComponentGL, data);
                if (texture->Format != sTexture::eFormat::DEPTH_STENCIL)
                {
                    if (texture->Format == sTexture::eFormat::RGBA8_MIPS)
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
            else if (texture->Type == sTexture::eType::TEXTURE_2D_ARRAY)
            {
                glBindTexture(GL_TEXTURE_2D_ARRAY, texture->Instance);
                glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, formatGL, texture->Width, texture->Height, texture->Depth, 0, channelsGL, formatComponentGL, data);
                
                if (texture->Format == sTexture::eFormat::RGBA8_MIPS)
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

        sTexture* cOpenGLRenderContext::ResizeTexture(sTexture* texture, const glm::vec2& size)
        {
            sTexture textureCopy = *texture;
            DestroyTexture(texture);

            sTexture* newTexture = CreateTexture(size.x, size.y, textureCopy.Depth, textureCopy.Type, textureCopy.Format, nullptr);
            
            return newTexture;
        }

        void cOpenGLRenderContext::BindTexture(const sShader* const shader, const std::string& name, const sTexture* const texture, s32 slot)
        {
            if (slot == -1)
                slot = texture->Slot;
            if (texture->Type == sTexture::eType::TEXTURE_2D)
            {
                glUniform1i(glGetUniformLocation(shader->Instance, name.c_str()), slot);
                glActiveTexture(GL_TEXTURE0 + slot);
                glBindTexture(GL_TEXTURE_2D, texture->Instance);
                glActiveTexture(GL_TEXTURE0);
            }
            else if (texture->Type == sTexture::eType::TEXTURE_2D_ARRAY)
            {
                glUniform1i(glGetUniformLocation(shader->Instance, name.c_str()), slot);
                glActiveTexture(GL_TEXTURE0 + slot);
                glBindTexture(GL_TEXTURE_2D_ARRAY, texture->Instance);
                glActiveTexture(GL_TEXTURE0);
            }
        }

        void cOpenGLRenderContext::UnbindTexture(const sTexture* const texture)
        {
            if (texture->Type == sTexture::eType::TEXTURE_2D_ARRAY)
                glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
        }

        void cOpenGLRenderContext::WriteTexture(sTexture* const texture, const glm::vec3& offset, const glm::vec2& size, const void* const data)
        {
            GLenum formatComponentGL = GL_UNSIGNED_BYTE;
            GLenum channelsGL = GL_RGBA;
            if (texture->Format == sTexture::eFormat::R8)
            {
                channelsGL = GL_RED;
                formatComponentGL = GL_UNSIGNED_BYTE;
            }
            else if (texture->Format == sTexture::eFormat::R8F)
            {
                channelsGL = GL_RED;
                formatComponentGL = GL_FLOAT;
            }
            else if (texture->Format == sTexture::eFormat::RGBA8)
            {
                channelsGL = GL_RGBA;
                formatComponentGL = GL_UNSIGNED_BYTE;
            }
            else if (texture->Format == sTexture::eFormat::RGB16F)
            {
                channelsGL = GL_RGB;
                formatComponentGL = GL_HALF_FLOAT;
            }
            else if (texture->Format == sTexture::eFormat::RGBA16F)
            {
                channelsGL = GL_RGBA;
                formatComponentGL = GL_HALF_FLOAT;
            }
            else if (texture->Format == sTexture::eFormat::DEPTH_STENCIL)
            {
                channelsGL = GL_DEPTH_STENCIL;
                formatComponentGL = GL_UNSIGNED_INT_24_8;
            }

            if (texture->Type == sTexture::eType::TEXTURE_2D)
            {
                glBindTexture(GL_TEXTURE_2D, texture->Instance);
                glTexSubImage2D(GL_TEXTURE_2D, 0, offset.x, offset.y, size.x, size.y, channelsGL, formatComponentGL, data);
                glBindTexture(GL_TEXTURE_2D, 0);
            }
            else if (texture->Type == sTexture::eType::TEXTURE_2D_ARRAY)
            {
                if (offset.x + size.x <= texture->Width && offset.y + size.y <= texture->Height && offset.z < texture->Depth)
                {
                    glBindTexture(GL_TEXTURE_2D_ARRAY, texture->Instance);
                    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, offset.x, offset.y, offset.z, size.x, size.y, 1, channelsGL, formatComponentGL, data);
                    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
                }
            }
        }

        void cOpenGLRenderContext::WriteTextureToFile(const sTexture* const texture, const std::string& filename)
        {
            if (texture->Format != sTexture::eFormat::RGBA8)
                return;

            GLenum channelsGL = GL_RGBA;
            GLenum formatComponentGL = GL_UNSIGNED_BYTE;
            usize formatByteCount = 4;
            if (texture->Format == sTexture::eFormat::RGBA8)
            {
                channelsGL = GL_RGBA;
                formatComponentGL = GL_UNSIGNED_BYTE;
                formatByteCount = 4;
            }

            if (texture->Type == sTexture::eType::TEXTURE_2D)
            {
                char* pixels = (char*)malloc(texture->Width * texture->Height * 4);

                glBindTexture(GL_TEXTURE_2D, texture->Instance);
                glGetTexImage(GL_TEXTURE_2D, 0, channelsGL, formatComponentGL, pixels);
                glBindTexture(GL_TEXTURE_2D, 0);

                lodepng_encode32_file(filename.c_str(), (const unsigned char*)pixels, texture->Width, texture->Height);

                free(pixels);
            }
        }

        void cOpenGLRenderContext::GenerateTextureMips(sTexture* const texture)
        {
            if (texture->Type == sTexture::eType::TEXTURE_2D)
            {
                glBindTexture(GL_TEXTURE_2D, texture->Instance);
                glGenerateMipmap(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, 0);
            }
            else if (texture->Type == sTexture::eType::TEXTURE_2D_ARRAY)
            {
                glBindTexture(GL_TEXTURE_2D_ARRAY, texture->Instance);
                glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
                glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
            }
        }

        void cOpenGLRenderContext::DestroyTexture(sTexture* texture)
        {
            if (texture->Type == sTexture::eType::TEXTURE_2D)
                glBindTexture(GL_TEXTURE_2D, 0);
            else if (texture->Type == sTexture::eType::TEXTURE_2D_ARRAY)
                glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

            glDeleteTextures(1, (GLuint*)&texture->Instance);

            if (texture != nullptr)
            {
                texture->~sTexture();
                _app->GetMemoryPool()->Free(texture);
            }
        }

        sRenderTarget* cOpenGLRenderContext::CreateRenderTarget(const std::vector<sTexture*>& colorAttachments, const sTexture* const depthAttachment)
        {
            sRenderTarget* pRenderTarget = (sRenderTarget*)_app->GetMemoryPool()->Allocate(sizeof(sRenderTarget));
            sRenderTarget* renderTarget = new (pRenderTarget) sRenderTarget();

            renderTarget->ColorAttachments = colorAttachments;
            renderTarget->DepthAttachment = (sTexture*)depthAttachment;

            GLenum buffs[16] = {};
            glGenFramebuffers(1, (GLuint*)&renderTarget->Instance);
            glBindFramebuffer(GL_FRAMEBUFFER, renderTarget->Instance);
            for (usize i = 0; i < renderTarget->ColorAttachments.size(); i++)
            {
                buffs[i] = GL_COLOR_ATTACHMENT0 + i;
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, renderTarget->ColorAttachments[i]->Instance, 0);
            }
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, renderTarget->DepthAttachment->Instance, 0);
            glDrawBuffers(renderTarget->ColorAttachments.size(), &buffs[0]);
            GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
			
			if (status != GL_FRAMEBUFFER_COMPLETE)
                std::cout << "Error: incomplete framebuffer!" << std::endl;

            return renderTarget;
        }

        void cOpenGLRenderContext::ResizeRenderTargetColors(sRenderTarget* const renderTarget, const glm::vec2& size)
        {
            std::vector<sTexture*> newColorAttachments;
            for (auto attachment : renderTarget->ColorAttachments)
            {
                sTexture attachmentCopy = *attachment;
                DestroyTexture(attachment);
                newColorAttachments.emplace_back(CreateTexture(size.x, size.y, attachmentCopy.Depth, attachmentCopy.Type, attachmentCopy.Format, nullptr));
            }
            renderTarget->ColorAttachments.clear();
            renderTarget->ColorAttachments = newColorAttachments;

            GLenum buffs[16] = {};
            glBindFramebuffer(GL_FRAMEBUFFER, renderTarget->Instance);
            for (usize i = 0; i < renderTarget->ColorAttachments.size(); i++)
            {
                buffs[i] = GL_COLOR_ATTACHMENT0 + i;
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, renderTarget->ColorAttachments[i]->Instance, 0);
            }
            glDrawBuffers(renderTarget->ColorAttachments.size(), &buffs[0]);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        void cOpenGLRenderContext::ResizeRenderTargetDepth(sRenderTarget* const renderTarget, const glm::vec2& size)
        {
            sTexture attachmentCopy = *renderTarget->DepthAttachment;
            DestroyTexture(renderTarget->DepthAttachment);

            sTexture* newDepthAttachment = CreateTexture(size.x, size.y, attachmentCopy.Depth, attachmentCopy.Type, attachmentCopy.Format, nullptr);
            renderTarget->DepthAttachment = newDepthAttachment;

            GLenum buffs[16] = {};
            glBindFramebuffer(GL_FRAMEBUFFER, renderTarget->Instance);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, renderTarget->DepthAttachment->Instance, 0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        void cOpenGLRenderContext::UpdateRenderTargetBuffers(const sRenderTarget* const renderTarget)
        {
            GLenum buffs[16] = {};
            glGenFramebuffers(1, (GLuint*)&renderTarget->Instance);
            glBindFramebuffer(GL_FRAMEBUFFER, renderTarget->Instance);
            for (usize i = 0; i < renderTarget->ColorAttachments.size(); i++)
            {
                buffs[i] = GL_COLOR_ATTACHMENT0 + i;
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, renderTarget->ColorAttachments[i]->Instance, 0);
            }
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, renderTarget->DepthAttachment->Instance, 0);
            glDrawBuffers(renderTarget->ColorAttachments.size(), &buffs[0]);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        void cOpenGLRenderContext::BindRenderTarget(const sRenderTarget* const renderTarget)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, renderTarget->Instance);
        }

        void cOpenGLRenderContext::UnbindRenderTarget()
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        void cOpenGLRenderContext::DestroyRenderTarget(sRenderTarget* renderTarget)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glDeleteFramebuffers(1, (GLuint*)&renderTarget->Instance);

            if (renderTarget != nullptr)
            {
                renderTarget->~sRenderTarget();
                _app->GetMemoryPool()->Free(renderTarget);
            }
        }

        sRenderPass* cOpenGLRenderContext::CreateRenderPass(const sRenderPass::sDescriptor& descriptor)
        {
            sRenderPass* pRenderPass = (sRenderPass*)_app->GetMemoryPool()->Allocate(sizeof(sRenderPass));
            sRenderPass* renderPass = new (pRenderPass) sRenderPass();

            memset(renderPass, 0, sizeof(sRenderPass));

            renderPass->Desc = descriptor;

            renderPass->Desc.VertexArray = CreateVertexArray();
            BindVertexArray(renderPass->Desc.VertexArray);
            if (renderPass->Desc.InputVertexFormat == Category::VERTEX_BUFFER_FORMAT_NONE)
            {
                for (auto buffer : renderPass->Desc.InputBuffers)
                    BindBuffer(buffer);
            }
            else if (renderPass->Desc.InputVertexFormat == Category::VERTEX_BUFFER_FORMAT_POS_TEX_NRM_VEC3_VEC2_VEC3)
            {
                for (auto buffer : renderPass->Desc.InputBuffers)
                    BindBuffer(buffer);

                BindDefaultInputLayout();
            }

            UnbindVertexArray();

            return renderPass;
        }

        void cOpenGLRenderContext::BindRenderPass(const sRenderPass* const renderPass, const sShader* const customShader)
        {
            sShader* shader = nullptr;
            if (customShader == nullptr)
                shader = renderPass->Desc.Shader;
            else
                shader = (sShader*)customShader;

            BindShader(shader);
            BindVertexArray(renderPass->Desc.VertexArray);
            if (renderPass->Desc.RenderTarget != nullptr)
                BindRenderTarget(renderPass->Desc.RenderTarget);
            else
                UnbindRenderTarget();
            Viewport(renderPass->Desc.Viewport);
            for (auto buffer : renderPass->Desc.InputBuffers)
                BindBufferNotVAO(buffer);
            BindDepthMode(renderPass->Desc.DepthMode);
            BindBlendMode(renderPass->Desc.BlendMode);
            for (usize i = 0; i < renderPass->Desc.InputTextures.size(); i++)
                BindTexture(shader, renderPass->Desc.InputTextureNames[i].c_str(), renderPass->Desc.InputTextures[i], i);
        }

        void cOpenGLRenderContext::UnbindRenderPass(const sRenderPass* const renderPass)
        {
            UnbindVertexArray();
            if (renderPass->Desc.RenderTarget != nullptr)
                UnbindRenderTarget();
            for (auto buffer : renderPass->Desc.InputBuffers)
                UnbindBuffer(buffer);
            for (auto texture : renderPass->Desc.InputTextures)
                UnbindTexture(texture);
        }

        void cOpenGLRenderContext::DestroyRenderPass(sRenderPass* renderPass)
        {
            glBindVertexArray(0);
            DestroyVertexArray(renderPass->Desc.VertexArray);

            if (renderPass != nullptr)
            {
                renderPass->~sRenderPass();
                _app->GetMemoryPool()->Free(renderPass);
            }
        }

        void cOpenGLRenderContext::BindDefaultInputLayout()
        {
            glEnableVertexAttribArray(0);
            glEnableVertexAttribArray(1);
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 32, (void*)0);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 32, (void*)12);
            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 32, (void*)20);
        }

        void cOpenGLRenderContext::BindBlendMode(const sBlendMode& blendMode)
        {
            for (usize i = 0; i < blendMode.FactorCount; i++)
            {
                GLuint srcFactor = GL_ZERO;
                GLuint dstFactor = GL_ZERO;

                switch (blendMode.SrcFactors[i])
                {
                    case sBlendMode::eFactor::ONE: srcFactor = GL_ONE; break;
                    case sBlendMode::eFactor::SRC_COLOR: srcFactor = GL_SRC_COLOR; break;
                    case sBlendMode::eFactor::INV_SRC_COLOR: srcFactor = GL_ONE_MINUS_SRC_COLOR; break;
                    case sBlendMode::eFactor::SRC_ALPHA: srcFactor = GL_SRC_ALPHA; break;
                    case sBlendMode::eFactor::INV_SRC_ALPHA: srcFactor = GL_ONE_MINUS_SRC_ALPHA; break;
                }

                switch (blendMode.DstFactors[i])
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

        void cOpenGLRenderContext::BindDepthMode(const sDepthMode& blendMode)
        {
            if (blendMode.UseDepthTest == K_TRUE)
                glEnable(GL_DEPTH_TEST);
            else
                glDisable(GL_DEPTH_TEST);

            if (blendMode.UseDepthWrite == K_TRUE)
                glDepthMask(GL_TRUE);
            else
                glDepthMask(GL_FALSE);
        }

        void cOpenGLRenderContext::Viewport(const glm::vec4& viewport)
        {
            glViewport(viewport.x, viewport.y, viewport.z, viewport.w);
        }

        void cOpenGLRenderContext::ClearColor(const glm::vec4& color)
        {
            glClearColor(color.x, color.y, color.z, color.w);
            glClear(GL_COLOR_BUFFER_BIT);
        }

        void cOpenGLRenderContext::ClearDepth(const f32 depth)
        {
            glClearDepth(depth);
            glClear(GL_DEPTH_BUFFER_BIT);
        }

        void cOpenGLRenderContext::ClearFramebufferColor(const usize bufferIndex, const glm::vec4& color)
        {
            glClearBufferfv(GL_COLOR, bufferIndex, &color.x);
        }

        void cOpenGLRenderContext::ClearFramebufferDepth(const f32 depth)
        {
            glClearDepth(depth);
            glClear(GL_DEPTH_BUFFER_BIT);
        }

        void cOpenGLRenderContext::Draw(usize indexCount, usize vertexOffset, usize indexOffset, usize instanceCount)
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

        void cOpenGLRenderContext::DrawQuad()
        {
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }

        void cOpenGLRenderContext::DrawQuads(usize count)
        {
            glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, count);
        }
    }
}