// graphics_resource_backend.cpp

#include "gl.hpp"
#include "graphics_resource_backend.hpp"
#include "context.hpp"
#include "thread_guard.hpp"

using namespace types;

triton::cBuffer::cBuffer(cContext* context, qword instance, eType type, types::usize byteSize, types::s32 slot) 
    : cGPUResource(context, instance, 0), _type(type), _byteSize(byteSize), _slot(slot) {}

void triton::cBuffer::Bind()
{
    CThreadGuard::AssertRender();

    _context->GetBackend<iGraphicsResourceBackend>()->BindBuffer(
        this
    );
}

void triton::cBuffer::Unbind()
{
    CThreadGuard::AssertRender();

    _context->GetBackend<iGraphicsResourceBackend>()->UnbindBuffer(
        this
    );
}

void triton::cBuffer::Write(types::usize byteOffset, types::u8* data, types::usize byteSize)
{
    CThreadGuard::AssertRender();

    _context->GetBackend<iGraphicsResourceBackend>()->WriteBuffer(
        this,
        byteOffset,
        byteSize,
        data
    );
}

triton::cTexture::cTexture(
    cContext* context,
    qword instance,
    const cVector3& size,
    eDimension dimension,
    eFormat format,
    s32 slot
) : cGPUResource(context, instance, 0), _size(size), _dimension(dimension), _format(format), _slot(slot) {}

triton::iGraphicsResourceBackend::iGraphicsResourceBackend(cContext* context) : iBackend(context) {}