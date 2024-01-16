#include <iostream>
#include <cstring>
#include <GL/glew.h>
#include <lodepng.h>

#include "render_context.hpp"
#include "filesystem_manager.hpp"
#include "types.hpp"

extern realware::core::mFileSystem* fileSystemManager;

namespace realware
{
    namespace render
    {
        cOpenGLRenderContext::cOpenGLRenderContext()
        {
            glewInit();
            glEnable(GL_DEPTH_TEST);
            //glEnable(GL_CULL_FACE);
            glEnable(GL_BLEND);
            glDepthFunc(GL_LESS);
            //glCullFace(GL_BACK);
            glFrontFace(GL_CW);
        }

        cOpenGLRenderContext::~cOpenGLRenderContext()
        {
        }

        sBuffer* cOpenGLRenderContext::CreateBuffer(core::usize byteSize, const sBuffer::eType& type, const void* data)
        {
            sBuffer* buffer = new sBuffer();
            buffer->ByteSize = byteSize;
            buffer->Type = type;
            buffer->Slot = 0;

            glGenBuffers(1, (GLuint*)&buffer->Instance);

            if (buffer->Type == sBuffer::eType::VERTEX)
            {
                glBindBuffer(GL_ARRAY_BUFFER, (GLuint)(core::u64)buffer->Instance);
                glBufferData(GL_ARRAY_BUFFER, byteSize, data, GL_STATIC_DRAW);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
            }
            else if (buffer->Type == sBuffer::eType::INDEX)
            {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (GLuint)(core::u64)buffer->Instance);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, byteSize, data, GL_STATIC_DRAW);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            }
            else if (buffer->Type == sBuffer::eType::UNIFORM)
            {
                glBindBuffer(GL_UNIFORM_BUFFER, (GLuint)(core::u64)buffer->Instance);
                glBufferData(GL_UNIFORM_BUFFER, byteSize, data, GL_STATIC_DRAW);
                glBindBuffer(GL_UNIFORM_BUFFER, 0);
            }
            else if (buffer->Type == sBuffer::eType::LARGE)
            {
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, (GLuint)(core::u64)buffer->Instance);
                glBufferData(GL_SHADER_STORAGE_BUFFER, byteSize, data, GL_STATIC_DRAW);
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
            }

            return buffer;
        }

        void cOpenGLRenderContext::BindBuffer(const sBuffer* buffer)
        {
            if (buffer->Type == sBuffer::eType::VERTEX) {
                glBindBuffer(GL_ARRAY_BUFFER, (GLuint)(core::u64)buffer->Instance);
            } else if (buffer->Type == sBuffer::eType::INDEX) {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (GLuint) (core::u64) buffer->Instance);
            } else if (buffer->Type == sBuffer::eType::UNIFORM) {
                glBindBufferBase(GL_UNIFORM_BUFFER, buffer->Slot, (GLuint)(core::u64)buffer->Instance);
            } else if (buffer->Type == sBuffer::eType::LARGE) {
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, buffer->Slot, buffer->Instance);
            }
        }

        void cOpenGLRenderContext::UnbindBuffer(const sBuffer* buffer)
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

        void cOpenGLRenderContext::WriteBuffer(sBuffer* buffer, core::usize offset, core::usize byteSize, const void* data)
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

        void cOpenGLRenderContext::DeleteBuffer(sBuffer* buffer)
        {
            glDeleteBuffers(1, (GLuint*)&buffer->Instance);
        }

        sVertexArray* cOpenGLRenderContext::CreateVertexArray()
        {
            sVertexArray* vertexArray = new sVertexArray();

            glGenVertexArrays(1, (GLuint*)&vertexArray->Instance);

            return vertexArray;
        }

        void cOpenGLRenderContext::BindVertexArray(const sVertexArray* vertexArray)
        {
            glBindVertexArray((GLuint)(core::u64)vertexArray->Instance);
        }

        void cOpenGLRenderContext::BindDefaultVertexArray(const std::vector<sBuffer*>& buffersToBind)
        {
            static sVertexArray* vertexArray = nullptr;

            if (vertexArray == nullptr)
            {
                vertexArray = CreateVertexArray();

                BindVertexArray(vertexArray);
                for (auto buffer : buffersToBind) {
                    BindBuffer(buffer);
                }
                BindDefaultInputLayout();
                UnbindVertexArray();
            }

            BindVertexArray(vertexArray);
        }

        void cOpenGLRenderContext::UnbindVertexArray()
        {
            glBindVertexArray(0);
        }

        void cOpenGLRenderContext::DeleteVertexArray(sVertexArray* vertexArray)
        {
            glDeleteVertexArrays(1, (GLuint*)&vertexArray->Instance);
        }

        void cOpenGLRenderContext::BindShader(const sShader* shader)
        {
            glUseProgram((GLuint)(core::u64)shader->Instance);
        }

        sShader* cOpenGLRenderContext::BindOpaqueShader()
        {
            static sShader* shader = nullptr;

            if (shader == nullptr)
            {
                shader = new sShader();

                core::sFile vertexShaderFile = fileSystemManager->LoadFile("data/shaders/opaque_vertex.shader", core::K_TRUE);
                const char* vertex = (const char*)vertexShaderFile.Data;

                core::sFile fragmentShaderFile = fileSystemManager->LoadFile("data/shaders/opaque_fragment.shader", core::K_TRUE);
                const char* fragment = (const char*)fragmentShaderFile.Data;

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

                GLint bl = 1024;
                char b[1024] = {};
                glGetShaderInfoLog(vertexShader, 1024, &bl, &b[0]);
                std::cout << b << std::endl;
                glGetShaderInfoLog(fragmentShader, 1024, &bl, &b[0]);
                std::cout << b << std::endl;

                fileSystemManager->DeleteFile(vertexShaderFile);
                fileSystemManager->DeleteFile(fragmentShaderFile);
            }

            glUseProgram(shader->Instance);

            return shader;
        }

        sShader* cOpenGLRenderContext::BindTransparentShader()
        {
            static sShader* shader = nullptr;

            if (shader == nullptr)
            {
                shader = new sShader();

                core::sFile vertexShaderFile = fileSystemManager->LoadFile("data/shaders/transparent_vertex.shader", core::K_TRUE);
                const char* vertex = (const char*)vertexShaderFile.Data;

                core::sFile fragmentShaderFile = fileSystemManager->LoadFile("data/shaders/transparent_fragment.shader", core::K_TRUE);
                const char* fragment = (const char*)fragmentShaderFile.Data;

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

                GLint bl = 1024;
                char b[1024] = {};
                glGetShaderInfoLog(vertexShader, 1024, &bl, &b[0]);
                std::cout << b << std::endl;
                glGetShaderInfoLog(fragmentShader, 1024, &bl, &b[0]);
                std::cout << b << std::endl;

                fileSystemManager->DeleteFile(vertexShaderFile);
                fileSystemManager->DeleteFile(fragmentShaderFile);
            }

            glUseProgram(shader->Instance);

            return shader;
        }

        sShader* cOpenGLRenderContext::BindQuadShader()
        {
            static sShader* shader = nullptr;

            if (shader == nullptr)
            {
                shader = new sShader();

                core::sFile vertexShaderFile = fileSystemManager->LoadFile("data/shaders/quad_vertex.shader", core::K_TRUE);
                const char* vertex = (const char*)vertexShaderFile.Data;

                core::sFile fragmentShaderFile = fileSystemManager->LoadFile("data/shaders/quad_fragment.shader", core::K_TRUE);
                const char* fragment = (const char*)fragmentShaderFile.Data;

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

                GLint bl = 1024;
                char b[1024] = {};
                glGetShaderInfoLog(vertexShader, 1024, &bl, &b[0]);
                std::cout << b << std::endl;
                glGetShaderInfoLog(fragmentShader, 1024, &bl, &b[0]);
                std::cout << b << std::endl;

                fileSystemManager->DeleteFile(vertexShaderFile);
                fileSystemManager->DeleteFile(fragmentShaderFile);
            }

            glUseProgram(shader->Instance);

            return shader;
        }

        sShader* cOpenGLRenderContext::BindTextShader()
        {
            static sShader* shader = nullptr;

            if (shader == nullptr)
            {
                shader = new sShader();

                core::sFile vertexShaderFile = fileSystemManager->LoadFile("data/shaders/text_vertex.shader", core::K_TRUE);
                const char* vertex = (const char*)vertexShaderFile.Data;

                core::sFile fragmentShaderFile = fileSystemManager->LoadFile("data/shaders/text_fragment.shader", core::K_TRUE);
                const char* fragment = (const char*)fragmentShaderFile.Data;

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

                GLint bl = 1024;
                char b[1024] = {};
                glGetShaderInfoLog(vertexShader, 1024, &bl, &b[0]);
                std::cout << b << std::endl;
                glGetShaderInfoLog(fragmentShader, 1024, &bl, &b[0]);
                std::cout << b << std::endl;

                fileSystemManager->DeleteFile(vertexShaderFile);
                fileSystemManager->DeleteFile(fragmentShaderFile);
            }

            glUseProgram(shader->Instance);

            return shader;
        }

        sShader* cOpenGLRenderContext::BindWidgetShader()
        {
            static sShader* shader = nullptr;

            if (shader == nullptr)
            {
                shader = new sShader();

                core::sFile vertexShaderFile = fileSystemManager->LoadFile("data/shaders/widget_vertex.shader", core::K_TRUE);
                const char* vertex = (const char*)vertexShaderFile.Data;

                core::sFile fragmentShaderFile = fileSystemManager->LoadFile("data/shaders/widget_fragment.shader", core::K_TRUE);
                const char* fragment = (const char*)fragmentShaderFile.Data;

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

                GLint bl = 1024;
                char b[1024] = {};
                glGetShaderInfoLog(vertexShader, 1024, &bl, &b[0]);
                std::cout << b << std::endl;
                glGetShaderInfoLog(fragmentShader, 1024, &bl, &b[0]);
                std::cout << b << std::endl;

                fileSystemManager->DeleteFile(vertexShaderFile);
                fileSystemManager->DeleteFile(fragmentShaderFile);
            }

            glUseProgram(shader->Instance);

            return shader;
        }

        sShader* cOpenGLRenderContext::BindCompositeTransparentShader()
        {
            static sShader* shader = nullptr;

            if (shader == nullptr)
            {
                shader = new sShader();

                core::sFile vertexShaderFile = fileSystemManager->LoadFile("data/shaders/composite_transparent_vertex.shader", core::K_TRUE);
                const char* vertex = (const char*)vertexShaderFile.Data;

                core::sFile fragmentShaderFile = fileSystemManager->LoadFile("data/shaders/composite_transparent_fragment.shader", core::K_TRUE);
                const char* fragment = (const char*)fragmentShaderFile.Data;

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

                GLint bl = 1024;
                char b[1024] = {};
                glGetShaderInfoLog(vertexShader, 1024, &bl, &b[0]);
                std::cout << b << std::endl;
                glGetShaderInfoLog(fragmentShader, 1024, &bl, &b[0]);
                std::cout << b << std::endl;

                fileSystemManager->DeleteFile(vertexShaderFile);
                fileSystemManager->DeleteFile(fragmentShaderFile);
            }

            glUseProgram(shader->Instance);

            return shader;
        }

        void cOpenGLRenderContext::SetShaderUniform(const sShader* shader, const char* name, const glm::mat4& matrix)
        {
            glUniformMatrix4fv(glGetUniformLocation(shader->Instance, name), 1, GL_FALSE, &matrix[0][0]);
        }

        void cOpenGLRenderContext::SetShaderUniform(const sShader* shader, const char* name, core::usize count, float* values)
        {
            glUniform4fv(glGetUniformLocation(shader->Instance, name), count, &values[0]);
        }

        sTexture* cOpenGLRenderContext::CreateTexture(core::s32 width, core::s32 height, core::s32 depth, const sTexture::eType &type, const sTexture::eFormat &format, const void *data)
        {
            sTexture* texture = new sTexture();
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
            else if (texture->Format == sTexture::eFormat::RGBA8)
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
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                }
                glBindTexture(GL_TEXTURE_2D, 0);
            }
            else if (texture->Type == sTexture::eType::TEXTURE_2D_ARRAY)
            {
                glBindTexture(GL_TEXTURE_2D_ARRAY, texture->Instance);
                glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, formatGL, texture->Width, texture->Height, texture->Depth, 0, channelsGL, formatComponentGL, data);
                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
            }

            return texture;
        }

        sTexture* cOpenGLRenderContext::ResizeTexture(sTexture* texture, const glm::vec2& size)
        {
            DeleteTexture(texture);
            sTexture* newTexture = CreateTexture(size.x, size.y, texture->Depth, texture->Type, texture->Format, nullptr);
            delete texture;

            return newTexture;
        }

        void cOpenGLRenderContext::BindTexture(const sShader* shader, const char* name, const sTexture* texture)
        {
            if (texture->Type == sTexture::eType::TEXTURE_2D)
            {
                glUniform1i(glGetUniformLocation(shader->Instance, name), texture->Slot);
                glActiveTexture(GL_TEXTURE0 + texture->Slot);
                glBindTexture(GL_TEXTURE_2D, texture->Instance);
                glActiveTexture(GL_TEXTURE0);
            }
            else if (texture->Type == sTexture::eType::TEXTURE_2D_ARRAY)
            {
                glUniform1i(glGetUniformLocation(shader->Instance, name), texture->Slot);
                glActiveTexture(GL_TEXTURE0 + texture->Slot);
                glBindTexture(GL_TEXTURE_2D_ARRAY, texture->Instance);
                glActiveTexture(GL_TEXTURE0);
            }
        }

        void cOpenGLRenderContext::UnbindTexture(const sTexture *texture)
        {
            if (texture->Type == sTexture::eType::TEXTURE_2D_ARRAY) {
                glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
            }
        }

        void cOpenGLRenderContext::WriteTexture(realware::render::sTexture* texture, const glm::vec3 &offset, const glm::vec2 &size, const void *data)
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

        void cOpenGLRenderContext::WriteTextureToFile(const sTexture* texture, const char* filename)
        {
            if (texture->Format != sTexture::eFormat::RGBA8) {
                return;
            }

            GLenum channelsGL = GL_RGBA;
            GLenum formatComponentGL = GL_UNSIGNED_BYTE;
            core::usize formatByteCount = 4;
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

                lodepng_encode32_file(filename, (const unsigned char*)pixels, texture->Width, texture->Height);

                free(pixels);
            }
        }

        void cOpenGLRenderContext::DeleteTexture(sTexture* texture)
        {
            glDeleteTextures(1, (GLuint*)&texture->Instance);
        }

        sRenderTarget* cOpenGLRenderContext::CreateRenderTarget(const std::vector<sTexture*>& colorAttachments, const sTexture* depthAttachment)
        {
            sRenderTarget* renderTarget = new sRenderTarget();
            renderTarget->ColorAttachments = colorAttachments;
            renderTarget->DepthAttachment = (sTexture*)depthAttachment;

            GLenum buffs[16] = {};
            glGenFramebuffers(1, (GLuint*)&renderTarget->Instance);
            glBindFramebuffer(GL_FRAMEBUFFER, renderTarget->Instance);
            for (core::s32 i = 0; i < renderTarget->ColorAttachments.size(); i++)
            {
                buffs[i] = GL_COLOR_ATTACHMENT0 + i;
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, renderTarget->ColorAttachments[i]->Instance, 0);
            }
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, renderTarget->DepthAttachment->Instance, 0);
            glDrawBuffers(renderTarget->ColorAttachments.size(), &buffs[0]);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            return renderTarget;
        }

        void cOpenGLRenderContext::ResizeRenderTargetColors(sRenderTarget* renderTarget, const glm::vec2& size)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            std::vector<sTexture*> newColorAttachments;
            for (auto attachment : renderTarget->ColorAttachments)
            {
                DeleteTexture(attachment);
                newColorAttachments.emplace_back(CreateTexture(size.x, size.y, attachment->Depth, attachment->Type, attachment->Format, nullptr));
                //delete attachment;
            }
            renderTarget->ColorAttachments.clear();
            renderTarget->ColorAttachments = newColorAttachments;

            GLenum buffs[16] = {};
            glBindFramebuffer(GL_FRAMEBUFFER, renderTarget->Instance);
            for (core::s32 i = 0; i < renderTarget->ColorAttachments.size(); i++)
            {
                buffs[i] = GL_COLOR_ATTACHMENT0 + i;
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, renderTarget->ColorAttachments[i]->Instance, 0);
            }
            glDrawBuffers(renderTarget->ColorAttachments.size(), &buffs[0]);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        void cOpenGLRenderContext::ResizeRenderTargetDepth(sRenderTarget* renderTarget, const glm::vec2& size)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            DeleteTexture(renderTarget->DepthAttachment);
            sTexture* newDepthAttachment = CreateTexture(size.x, size.y, renderTarget->DepthAttachment->Depth, renderTarget->DepthAttachment->Type, renderTarget->DepthAttachment->Format, nullptr);
            delete renderTarget->DepthAttachment;
            renderTarget->DepthAttachment = newDepthAttachment;

            GLenum buffs[16] = {};
            glBindFramebuffer(GL_FRAMEBUFFER, renderTarget->Instance);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, renderTarget->DepthAttachment->Instance, 0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        void cOpenGLRenderContext::UpdateRenderTargetBuffers(sRenderTarget* renderTarget)
        {
            GLenum buffs[16] = {};
            glGenFramebuffers(1, (GLuint*)&renderTarget->Instance);
            glBindFramebuffer(GL_FRAMEBUFFER, renderTarget->Instance);
            for (core::s32 i = 0; i < renderTarget->ColorAttachments.size(); i++)
            {
                buffs[i] = GL_COLOR_ATTACHMENT0 + i;
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, renderTarget->ColorAttachments[i]->Instance, 0);
            }
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, renderTarget->DepthAttachment->Instance, 0);
            glDrawBuffers(renderTarget->ColorAttachments.size(), &buffs[0]);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        void cOpenGLRenderContext::BindRenderTarget(const sRenderTarget* renderTarget)
        {
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, renderTarget->Instance);
        }

        void cOpenGLRenderContext::UnbindRenderTarget()
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        void cOpenGLRenderContext::DeleteRenderTarget(sRenderTarget* renderTarget)
        {
            glDeleteFramebuffers(1, (GLuint*)&renderTarget->Instance);
        }

        sRenderPass* cOpenGLRenderContext::CreateRenderPass(const sRenderPass::sDescriptor& descriptor)
        {
            sRenderPass* renderPass = new sRenderPass();
            memset(renderPass, 0, sizeof(sRenderPass));

            renderPass->Desc = descriptor;

            renderPass->Desc.VertexArray = CreateVertexArray();
            BindVertexArray(renderPass->Desc.VertexArray);
            if (renderPass->Desc.InputVertexFormat == sVertexBufferGeometry::eFormat::NONE)
            {
                for (auto buffer : renderPass->Desc.InputBuffers) {
                    BindBuffer(buffer);
                }
            }
            else if (renderPass->Desc.InputVertexFormat == sVertexBufferGeometry::eFormat::POSITION_TEXCOORD_NORMAL_VEC3_VEC2_VEC3)
            {
                for (auto buffer : renderPass->Desc.InputBuffers) {
                    BindBuffer(buffer);
                }
                BindDefaultInputLayout();
            }
            UnbindVertexArray();

            return renderPass;
        }

        void cOpenGLRenderContext::BindRenderPass(const sRenderPass* renderPass)
        {
            BindShader(renderPass->Desc.Shader);
            BindVertexArray(renderPass->Desc.VertexArray);
            if (renderPass->Desc.RenderTarget != nullptr) {
                BindRenderTarget(renderPass->Desc.RenderTarget);
            } else {
                UnbindRenderTarget();
            }
            Viewport(renderPass->Desc.Viewport);
            for (auto buffer : renderPass->Desc.InputBuffers) {
                BindBuffer(buffer);
            }
            BindDepthMode(renderPass->Desc.DepthMode);
            BindBlendMode(renderPass->Desc.BlendMode);
            for (core::s32 i = 0; i < renderPass->Desc.InputTextures.size(); i++) {
                BindTexture(renderPass->Desc.Shader, renderPass->Desc.InputTextureNames[i].c_str(), renderPass->Desc.InputTextures[i]);
            }
        }

        void cOpenGLRenderContext::UnbindRenderPass(const sRenderPass* renderPass)
        {
            UnbindVertexArray();
            if (renderPass->Desc.RenderTarget != nullptr) {
                UnbindRenderTarget();
            }
            for (auto buffer : renderPass->Desc.InputBuffers) {
                UnbindBuffer(buffer);
            }
            for (auto texture : renderPass->Desc.InputTextures) {
                UnbindTexture(texture);
            }
        }

        void cOpenGLRenderContext::DeleteRenderPass(sRenderPass* renderPass)
        {
            DeleteVertexArray(renderPass->Desc.VertexArray);
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

        void cOpenGLRenderContext::BindDepthMode(const sDepthMode &blendMode)
        {
            if (blendMode.UseDepthTest == core::K_TRUE) {
                glEnable(GL_DEPTH_TEST);
            } else {
                glDisable(GL_DEPTH_TEST);
            }

            if (blendMode.UseDepthWrite == core::K_TRUE) {
                glDepthMask(GL_TRUE);
            } else {
                glDepthMask(GL_FALSE);
            }
        }

        void cOpenGLRenderContext::BindBlendMode(const sBlendMode& blendMode)
        {
            for (core::s32 i = 0; i < blendMode.FactorCount; i++)
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
        void cOpenGLRenderContext::Viewport(const glm::vec4& viewport)
        {
            glViewport(viewport.x, viewport.y, viewport.z, viewport.w);
        }

        void cOpenGLRenderContext::ClearColor(core::usize bufferIndex, const glm::vec4& color)
        {
            glClearBufferfv(GL_COLOR, bufferIndex, &color.x);
        }

        void cOpenGLRenderContext::ClearDepth(float depth)
        {
            glClearDepth(depth);
            glClear(GL_DEPTH_BUFFER_BIT);
        }

        void cOpenGLRenderContext::Draw(core::usize indexCount, core::usize vertexOffset, core::usize indexOffset, core::usize instanceCount)
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

        void cOpenGLRenderContext::DrawQuads(core::usize count)
        {
            glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, count);
        }
    }
}