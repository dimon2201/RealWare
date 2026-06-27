// render_batch.cpp

#include "render_batch.hpp"
#include "context.hpp"
#include "instance_buffer.hpp"
#include "handle_allocator.hpp"

triton::XRenderBatch::XRenderBatch(cContext* context, const SGeometryView& geometry) : iObject(context), _geometry(geometry)
{
	_staticInstances = _context->Create<XHandleAllocator<SInstanceBufferSlot, SInstanceBufferHandle, XLinearArray<SRenderInstance>, SRenderInstance>>(_context);
	_staticInstances->Initialize();
	_dynamicInstances = _context->Create<XHandleAllocator<SInstanceBufferSlot, SInstanceBufferHandle, XLinearArray<SRenderInstance>, SRenderInstance>>(_context);
	_dynamicInstances->Initialize();
}

triton::XRenderBatch::~XRenderBatch()
{
	_dynamicInstances->Free();
	_context->Destroy<XHandleAllocator<SInstanceBufferSlot, SInstanceBufferHandle, XLinearArray<SRenderInstance>, SRenderInstance>>(_dynamicInstances);
	_staticInstances->Free();
	_context->Destroy<XHandleAllocator<SInstanceBufferSlot, SInstanceBufferHandle, XLinearArray<SRenderInstance>, SRenderInstance>>(_staticInstances);
}

std::optional<triton::SInstanceBufferHandle> triton::XRenderBatch::Add(const SRenderBatchHandle& batch, SRenderInstance::EUsage usage, SRenderInstance& instance)
{
	if (usage == SRenderInstance::EUsage::STATIC)
		return _staticInstances->Create(SInstanceBufferHandle(batch, usage), instance);
	else if (usage == SRenderInstance::EUsage::DYNAMIC)
		return _dynamicInstances->Create(SInstanceBufferHandle(batch, usage), instance);

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

void triton::XRenderBatch::Write(SRenderInstance::EUsage usage, types::usize offset, types::u8* destination)
{
	if (usage == SRenderInstance::EUsage::STATIC)
	{
		_staticOffset = offset;
		SBufferView bufferView = _staticInstances->GetData();
		memcpy(&destination[0], bufferView._data, bufferView._byteSize);
	}
	else if (usage == SRenderInstance::EUsage::DYNAMIC)
	{
		_dynamicOffset = offset;
		SBufferView bufferView = _dynamicInstances->GetData();
		memcpy(&destination[0], bufferView._data, bufferView._byteSize);
	}
}