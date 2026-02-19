// graphics_resource_backend.cpp

#include <GL/glew.h>
#include "graphics_resource_backend.hpp"
#include "context.hpp"

using namespace types;

triton::cBuffer::cBuffer(cContext* context, qword instance, eType type, types::usize byteSize, types::s32 slot) 
    : cGPUResource(context, instance, 0), _type(type), _byteSize(byteSize), _slot(slot) {}

triton::cTexture::cTexture(
    cContext* context,
    qword instance,
    const cVector3& size,
    eDimension dimension,
    eFormat format,
    s32 slot
) : cGPUResource(context, instance, 0), _size(size), _dimension(dimension), _format(format), _slot(slot) {}

triton::iGraphicsResourceBackend::iGraphicsResourceBackend(cContext* context) : iBackend(context) {}