// render_batch.cpp

#include "render_batch.hpp"
#include "context.hpp"
#include "instance_buffer.hpp"

triton::XRenderBatch::XRenderBatch(cContext* context, const SGeometryView& geometry) : iObject(context), _geometry(geometry)
{
	_staticInstances = _context->Create<XInstanceBuffer>(_context, SRenderInstance::EUsage::STATIC);
	_dynamicInstances = _context->Create<XInstanceBuffer>(_context, SRenderInstance::EUsage::DYNAMIC);
}

triton::XRenderBatch::~XRenderBatch()
{
	_context->Destroy<XInstanceBuffer>(_dynamicInstances);
	_context->Destroy<XInstanceBuffer>(_staticInstances);
}

std::optional<triton::SInstanceBufferHandle> triton::XRenderBatch::Add(SRenderInstance::EUsage usage, SRenderInstance& instance)
{
	if (usage == SRenderInstance::EUsage::STATIC)
	{
		SInstanceBufferHandle ibh = _staticInstances->Add(instance);
		ibh._usage = SRenderInstance::EUsage::STATIC;

		return ibh;
	}
	else if (usage == SRenderInstance::EUsage::DYNAMIC)
	{
		SInstanceBufferHandle ibh = _dynamicInstances->Add(instance);
		ibh._usage = SRenderInstance::EUsage::DYNAMIC;

		return ibh;
	}

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
		return _staticInstances->Remove(handle);
	else if (handle._usage == SRenderInstance::EUsage::DYNAMIC)
		return _dynamicInstances->Remove(handle);
}