// graphics_buffer_backend.cpp

#include <GL/glew.h>
#include "graphics_buffer_backend.hpp"
#include "context.hpp"

using namespace types;

triton::cBuffer::cBuffer(cContext* context, qword instance, eType type, types::usize byteSize, types::s32 slot) 
    : cGPUResource(context, instance, 0), _type(type), _byteSize(byteSize), _slot(slot) {}

triton::iGraphicsBufferBackend::iGraphicsBufferBackend(cContext* context) : iBackend(context) {}