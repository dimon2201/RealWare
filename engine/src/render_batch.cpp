// render_batch.cpp

#include "render_batch.hpp"
#include "context.hpp"
#include "instance_buffer.hpp"

triton::XRenderBatch::XRenderBatch(cContext* context, const SGeometryView& geometry) : iObject(context), _geometry(geometry)
{
	_staticInstances = _context->Create<XInstanceBuffer>(_context);
	_dynamicInstances = _context->Create<XInstanceBuffer>(_context);
}

triton::XRenderBatch::~XRenderBatch()
{
	_context->Destroy<XInstanceBuffer>(_dynamicInstances);
	_context->Destroy<XInstanceBuffer>(_staticInstances);
}