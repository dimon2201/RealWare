// graphics_ogl_buffer_backend.cpp

#include <GL/glew.h>
#include "graphics_ogl_buffer_backend.hpp"
#include "context.hpp"

using namespace types;

triton::cGraphicsOGLBufferBackend::cGraphicsOGLBufferBackend(cContext* context) : iGraphicsBufferBackend(context) {}

triton::cBuffer* triton::cGraphicsOGLBufferBackend::CreateBuffer(
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

void triton::cGraphicsOGLBufferBackend::BindBuffer(const cBuffer* buffer)
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

void triton::cGraphicsOGLBufferBackend::BindBufferNotVAO(const cBuffer* buffer)
{
    if (buffer->GetBufferType() == cBuffer::eType::UNIFORM)
        glBindBufferBase(GL_UNIFORM_BUFFER, buffer->GetSlot(), (GLuint)buffer->GetInstance());
    else if (buffer->GetBufferType() == cBuffer::eType::LARGE)
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, buffer->GetSlot(), buffer->GetInstance());
}

void triton::cGraphicsOGLBufferBackend::UnbindBuffer(const cBuffer* buffer)
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

void triton::cGraphicsOGLBufferBackend::WriteBuffer(
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

void triton::cGraphicsOGLBufferBackend::DestroyBuffer(cBuffer* buffer)
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