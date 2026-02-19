// graphics_resource_backend_ogl.cpp

#include <GL/glew.h>
#include <lodepng.h>
#include "graphics_resource_backend_ogl.hpp"
#include "context.hpp"

using namespace types;

triton::cGraphicsResourceBackendOGL::cGraphicsResourceBackendOGL(cContext* context)
    : iGraphicsResourceBackend(context) {}

triton::cBuffer* triton::cGraphicsResourceBackendOGL::CreateBuffer(
    cBuffer::eType type,
    const u8* data,
    usize byteSize,
    s32 slot
)
{
    GLuint instance = 0;

    glGenBuffers(1, (GLuint*)&instance);

    if (type == cBuffer::eType::VERTEX)
    {
        glBindBuffer(GL_ARRAY_BUFFER, instance);
        glBufferData(GL_ARRAY_BUFFER, byteSize, data, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    else if (type == cBuffer::eType::INDEX)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, instance);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, byteSize, data, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    else if (type == cBuffer::eType::UNIFORM)
    {
        glBindBuffer(GL_UNIFORM_BUFFER, instance);
        glBufferData(GL_UNIFORM_BUFFER, byteSize, data, GL_STATIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }
    else if (type == cBuffer::eType::LARGE)
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, instance);
        glBufferData(GL_SHADER_STORAGE_BUFFER, byteSize, data, GL_STATIC_DRAW);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    cBuffer* buffer = _context->Create<cBuffer>(_context, instance, type, byteSize, slot);

    return buffer;
}

void triton::cGraphicsResourceBackendOGL::BindBuffer(const cBuffer* buffer)
{
    if (buffer->GetBufferType() == cBuffer::eType::VERTEX)
        glBindBuffer(GL_ARRAY_BUFFER, (GLuint)buffer->GetInstance());
    else if (buffer->GetBufferType() == cBuffer::eType::INDEX)
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (GLuint)buffer->GetInstance());
    else if (buffer->GetBufferType() == cBuffer::eType::UNIFORM)
        glBindBufferBase(GL_UNIFORM_BUFFER, buffer->GetSlot(), (GLuint)buffer->GetInstance());
    else if (buffer->GetBufferType() == cBuffer::eType::LARGE)
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, buffer->GetSlot(), buffer->GetInstance());
}

void triton::cGraphicsResourceBackendOGL::BindBufferNotVAO(const cBuffer* buffer)
{
    if (buffer->GetBufferType() == cBuffer::eType::UNIFORM)
        glBindBufferBase(GL_UNIFORM_BUFFER, buffer->GetSlot(), (GLuint)buffer->GetInstance());
    else if (buffer->GetBufferType() == cBuffer::eType::LARGE)
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, buffer->GetSlot(), buffer->GetInstance());
}

void triton::cGraphicsResourceBackendOGL::UnbindBuffer(const cBuffer* buffer)
{
    if (buffer->GetBufferType() == cBuffer::eType::VERTEX)
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    else if (buffer->GetBufferType() == cBuffer::eType::INDEX)
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    else if (buffer->GetBufferType() == cBuffer::eType::UNIFORM)
        glBindBufferBase(GL_UNIFORM_BUFFER, buffer->GetSlot(), 0);
    else if (buffer->GetBufferType() == cBuffer::eType::LARGE)
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, buffer->GetSlot(), 0);
}

void triton::cGraphicsResourceBackendOGL::WriteBuffer(
    const cBuffer* buffer,
    usize offset,
    usize byteSize,
    const types::u8* data
)
{
    if (buffer->GetBufferType() == cBuffer::eType::VERTEX)
    {
        glBindBuffer(GL_ARRAY_BUFFER, buffer->GetInstance());
        glBufferSubData(GL_ARRAY_BUFFER, offset, byteSize, data);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    else if (buffer->GetBufferType() == cBuffer::eType::INDEX)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->GetInstance());
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, byteSize, data);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    else if (buffer->GetBufferType() == cBuffer::eType::UNIFORM)
    {
        glBindBuffer(GL_UNIFORM_BUFFER, buffer->GetInstance());
        glBufferSubData(GL_UNIFORM_BUFFER, offset, byteSize, data);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }
    else if (buffer->GetBufferType() == cBuffer::eType::LARGE)
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer->GetInstance());
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, byteSize, data);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }
}

void triton::cGraphicsResourceBackendOGL::DestroyBuffer(cBuffer* buffer)
{
    if (buffer->GetBufferType() == cBuffer::eType::VERTEX)
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    else if (buffer->GetBufferType() == cBuffer::eType::INDEX)
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    else if (buffer->GetBufferType() == cBuffer::eType::UNIFORM)
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    else if (buffer->GetBufferType() == cBuffer::eType::LARGE)
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    GLuint instance = buffer->GetInstance();
    glDeleteBuffers(1, &instance);

    if (buffer != nullptr)
        _context->Destroy<cBuffer>(buffer);
}

triton::cTexture* triton::cGraphicsResourceBackendOGL::CreateTexture(
    const cVector3& size,
    cTexture::eDimension dimension,
    cTexture::eFormat format,
    const u8* data,
    s32 slot
)
{
    GLuint instance = 0;

    glGenTextures(1, (GLuint*)&instance);

    GLenum formatGL = GL_RGBA8;
    GLenum channelsGL = GL_RGBA;
    GLenum formatComponentGL = GL_UNSIGNED_BYTE;
    if (format == cTexture::eFormat::R8)
    {
        formatGL = GL_R8;
        channelsGL = GL_RED;
        formatComponentGL = GL_UNSIGNED_BYTE;
    }
    else if (format == cTexture::eFormat::R8F)
    {
        formatGL = GL_R8;
        channelsGL = GL_RED;
        formatComponentGL = GL_FLOAT;
    }
    else if (format == cTexture::eFormat::RGBA8 || format == cTexture::eFormat::RGBA8_MIPS)
    {
        formatGL = GL_RGBA8;
        channelsGL = GL_RGBA;
        formatComponentGL = GL_UNSIGNED_BYTE;
    }
    else if (format == cTexture::eFormat::RGB16F)
    {
        formatGL = GL_RGB16F;
        channelsGL = GL_RGB;
        formatComponentGL = GL_HALF_FLOAT;
    }
    else if (format == cTexture::eFormat::RGBA16F)
    {
        formatGL = GL_RGBA16F;
        channelsGL = GL_RGBA;
        formatComponentGL = GL_HALF_FLOAT;
    }
    else if (format == cTexture::eFormat::DEPTH_STENCIL)
    {
        formatGL = GL_DEPTH24_STENCIL8;
        channelsGL = GL_DEPTH_STENCIL;
        formatComponentGL = GL_UNSIGNED_INT_24_8;
    }

    if (dimension == cTexture::eDimension::TEXTURE_2D)
    {
        glBindTexture(GL_TEXTURE_2D, instance);

        glTexImage2D(GL_TEXTURE_2D, 0, formatGL, size.GetX(), size.GetY(), 0, channelsGL, formatComponentGL, data);
        if (format != cTexture::eFormat::DEPTH_STENCIL)
        {
            if (format == cTexture::eFormat::RGBA8_MIPS)
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
    else if (dimension == cTexture::eDimension::TEXTURE_2D_ARRAY)
    {
        glBindTexture(GL_TEXTURE_2D_ARRAY, instance);

        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, formatGL, size.GetX(), size.GetY(), size.GetZ(), 0, channelsGL, formatComponentGL, data);

        if (format == cTexture::eFormat::RGBA8_MIPS)
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

    cTexture* texture = _context->Create<cTexture>(_context, instance, size, dimension, format, slot);

    return texture;
}

triton::cTexture* triton::cGraphicsResourceBackendOGL::ResizeTexture(cTexture* texture, const cVector2& size)
{
    cTexture* newTexture = CreateTexture(
        cVector3(size.GetX(), size.GetY(), texture->GetDepth()),
        texture->GetDimension(),
        texture->GetFormat(),
        nullptr,
        texture->GetSlot()
    );
    DestroyTexture(texture);

    return newTexture;
}

void triton::cGraphicsResourceBackendOGL::BindTexture(const cTexture* texture)
{
    if (texture->GetDimension() == cTexture::eDimension::TEXTURE_2D)
    {
        glActiveTexture(GL_TEXTURE0 + texture->GetSlot());
        glBindTexture(GL_TEXTURE_2D, texture->GetInstance());
        glActiveTexture(GL_TEXTURE0);
    }
    else if (texture->GetDimension() == cTexture::eDimension::TEXTURE_2D_ARRAY)
    {
        glActiveTexture(GL_TEXTURE0 + texture->GetSlot());
        glBindTexture(GL_TEXTURE_2D_ARRAY, texture->GetInstance());
        glActiveTexture(GL_TEXTURE0);
    }

    // FIXME: figure out what's going on here
    // ||||||||||||||||||||||||||||||||||||||
    // ||||||||||||||||||||||||||||||||||||||
    // VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
    /*if (slot == -1)
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
    }*/
}

void triton::cGraphicsResourceBackendOGL::UnbindTexture(const cTexture* texture)
{
    if (texture->GetDimension() == cTexture::eDimension::TEXTURE_2D_ARRAY)
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void triton::cGraphicsResourceBackendOGL::WriteTexture(
    const cTexture* texture,
    const cVector3& offset,
    const cVector2& size,
    const u8* data
)
{
    GLenum formatGL = GL_RGBA8;
    GLenum channelsGL = GL_RGBA;
    GLenum formatComponentGL = GL_UNSIGNED_BYTE;

    if (texture->GetFormat() == cTexture::eFormat::R8)
    {
        formatGL = GL_R8;
        channelsGL = GL_RED;
        formatComponentGL = GL_UNSIGNED_BYTE;
    }
    else if (texture->GetFormat() == cTexture::eFormat::R8F)
    {
        formatGL = GL_R8;
        channelsGL = GL_RED;
        formatComponentGL = GL_FLOAT;
    }
    else if (
        texture->GetFormat() == cTexture::eFormat::RGBA8 ||
        texture->GetFormat() == cTexture::eFormat::RGBA8_MIPS
        )
    {
        formatGL = GL_RGBA8;
        channelsGL = GL_RGBA;
        formatComponentGL = GL_UNSIGNED_BYTE;
    }
    else if (texture->GetFormat() == cTexture::eFormat::RGB16F)
    {
        formatGL = GL_RGB16F;
        channelsGL = GL_RGB;
        formatComponentGL = GL_HALF_FLOAT;
    }
    else if (texture->GetFormat() == cTexture::eFormat::RGBA16F)
    {
        formatGL = GL_RGBA16F;
        channelsGL = GL_RGBA;
        formatComponentGL = GL_HALF_FLOAT;
    }
    else if (texture->GetFormat() == cTexture::eFormat::DEPTH_STENCIL)
    {
        formatGL = GL_DEPTH24_STENCIL8;
        channelsGL = GL_DEPTH_STENCIL;
        formatComponentGL = GL_UNSIGNED_INT_24_8;
    }

    if (texture->GetDimension() == cTexture::eDimension::TEXTURE_2D)
    {
        glBindTexture(GL_TEXTURE_2D, texture->GetInstance());
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
    else if (texture->GetDimension() == cTexture::eDimension::TEXTURE_2D_ARRAY)
    {
        if (
            offset.GetX() + size.GetX() <= texture->GetWidth() &&
            offset.GetY() + size.GetY() <= texture->GetHeight() &&
            offset.GetZ() < texture->GetDepth()
            )
        {
            glBindTexture(GL_TEXTURE_2D_ARRAY, texture->GetInstance());
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

void triton::cGraphicsResourceBackendOGL::WriteTextureToFile(const cTexture* texture, const std::string& filename)
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
        const sCapabilities* caps = _context->GetSubsystem<cEngine>()->GetCapabilities();
        cMemoryAllocator* memoryAllocator = _context->GetMemoryAllocator();

        u8* pixels = (u8*)memoryAllocator->Allocate(texture->GetWidth() * texture->GetHeight() * formatByteCount, caps->memoryAlignment);

        glBindTexture(GL_TEXTURE_2D, texture->GetInstance());
        glGetTexImage(GL_TEXTURE_2D, 0, channelsGL, formatComponentGL, pixels);
        glBindTexture(GL_TEXTURE_2D, 0);

        lodepng_encode32_file(filename.c_str(), pixels, texture->GetWidth(), texture->GetHeight());

        memoryAllocator->Deallocate(pixels);
    }
}

void triton::cGraphicsResourceBackendOGL::GenerateTextureMips(const cTexture* texture)
{
    if (texture->GetDimension() == cTexture::eDimension::TEXTURE_2D)
    {
        glBindTexture(GL_TEXTURE_2D, texture->GetInstance());
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    else if (texture->GetDimension() == cTexture::eDimension::TEXTURE_2D_ARRAY)
    {
        glBindTexture(GL_TEXTURE_2D_ARRAY, texture->GetInstance());
        glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    }
}

void triton::cGraphicsResourceBackendOGL::DestroyTexture(cTexture* texture)
{
    if (texture->GetDimension() == cTexture::eDimension::TEXTURE_2D)
        glBindTexture(GL_TEXTURE_2D, 0);
    else if (texture->GetDimension() == cTexture::eDimension::TEXTURE_2D_ARRAY)
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

    GLuint instance = texture->GetInstance();
    glDeleteTextures(1, (GLuint*)&instance);

    if (texture != nullptr)
        _context->Destroy<cTexture>(texture);
}