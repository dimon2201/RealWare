// graphics_texture_backend.cpp

#include "graphics_texture_backend.hpp"
#include "context.hpp"

using namespace types;

triton::cTexture::cTexture(
    cContext* context,
    qword instance,
    const cVector3& size,
    eDimension dimension,
    eFormat format,
    s32 slot
) : cGPUResource(context, instance, 0), _size(size), _dimension(dimension), _format(format), _slot(slot) {}

triton::iGraphicsTextureBackend::iGraphicsTextureBackend(cContext* context) : iBackend(context) {}