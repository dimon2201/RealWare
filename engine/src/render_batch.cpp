// render_batch.cpp

#include "render_batch.hpp"
#include "context.hpp"
#include "instance_buffer.hpp"
#include "handle_allocator.hpp"

triton::XRenderBatch::XRenderBatch(cContext* context, const SGeometryView& geometry) : iObject(context), _geometry(geometry)
{
	_staticInstances = _context->Create<XHandleAllocator<SInstanceBufferSlot, SInstanceBufferHandle, XLinearArray<SRenderInstance>, SRenderInstance>>(_context);
	_dynamicInstances = _context->Create<XHandleAllocator<SInstanceBufferSlot, SInstanceBufferHandle, XLinearArray<SRenderInstance>, SRenderInstance>>(_context);
	_staticInstanceBuffer = _context->Create<XInstanceBuffer>(_context, SRenderInstance::EUsage::STATIC);
	_dynamicInstanceBuffer = _context->Create<XInstanceBuffer>(_context, SRenderInstance::EUsage::DYNAMIC);
}

triton::XRenderBatch::~XRenderBatch()
{
	_context->Destroy<XInstanceBuffer>(_dynamicInstanceBuffer);
	_context->Destroy<XInstanceBuffer>(_staticInstanceBuffer);
	_context->Destroy<XHandleAllocator<SInstanceBufferSlot, SInstanceBufferHandle, XLinearArray<SRenderInstance>, SRenderInstance>>(_dynamicInstances);
	_context->Destroy<XHandleAllocator<SInstanceBufferSlot, SInstanceBufferHandle, XLinearArray<SRenderInstance>, SRenderInstance>>(_staticInstances);
}

std::optional<triton::SInstanceBufferHandle> triton::XRenderBatch::Add(SRenderInstance::EUsage usage, SRenderInstance& instance)
{
	if (usage == SRenderInstance::EUsage::STATIC)
		return _staticInstances->Create(SInstanceBufferHandle(usage), instance);
	else if (usage == SRenderInstance::EUsage::DYNAMIC)
		return _dynamicInstances->Create(SInstanceBufferHandle(usage), instance);

	return std::nullopt;
}

triton::SRenderInstance* triton::XRenderBatch::Get(SInstanceBufferHandle& handle)
{
	if (handle._usage == SRenderInstance::EUsage::STATIC)
		return _staticInstances->Get(handle);
	else if (handle._usage == SRenderInstance::EUsage::DYNAMIC)
		return _dynamicInstances->Get(handle);

	return nullptr;
}

void triton::XRenderBatch::Remove(SInstanceBufferHandle& handle)
{
	if (handle._usage == SRenderInstance::EUsage::STATIC)
		return _staticInstances->Destroy(handle);
	else if (handle._usage == SRenderInstance::EUsage::DYNAMIC)
		return _dynamicInstances->Destroy(handle);
}