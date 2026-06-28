// render_batch.cpp

#include "render_batch.hpp"
#include "context.hpp"
#include "instance_buffer.hpp"
#include "handle_allocator.hpp"

using namespace types;

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
		return _staticInstances->Create(SInstanceBufferHandle(batch, usage), std::move(instance));
	else if (usage == SRenderInstance::EUsage::DYNAMIC)
		return _dynamicInstances->Create(SInstanceBufferHandle(batch, usage), std::move(instance));

	return std::nullopt;
}

triton::SRenderInstance* triton::XRenderBatch::Get(const SInstanceBufferHandle& handle)
{
	if (handle._usage == SRenderInstance::EUsage::STATIC)
		return _staticInstances->Get(handle);
	else if (handle._usage == SRenderInstance::EUsage::DYNAMIC)
		return _dynamicInstances->Get(handle);

	return nullptr;
}

void triton::XRenderBatch::Set(const SInstanceBufferHandle& handle, const SRenderInstance& instance)
{
	if (handle._usage == SRenderInstance::EUsage::STATIC)
		*_staticInstances->Get(handle) = instance;
	else if (handle._usage == SRenderInstance::EUsage::DYNAMIC)
		*_dynamicInstances->Get(handle) = instance;
}

void triton::XRenderBatch::Remove(const SInstanceBufferHandle& handle)
{
	if (handle._usage == SRenderInstance::EUsage::STATIC)
		return _staticInstances->Destroy(handle);
	else if (handle._usage == SRenderInstance::EUsage::DYNAMIC)
		return _dynamicInstances->Destroy(handle);
}

types::usize triton::XRenderBatch::Write(SRenderInstance::EUsage usage, types::usize offset, types::u8* destination)
{
	usize nextOffset;
	if (usage == SRenderInstance::EUsage::STATIC)
	{
		_staticOffset = offset;
		SBufferView bufferView = _staticInstances->GetData();
		memcpy(&destination[offset], bufferView._elements, bufferView._byteSize);
		nextOffset = offset + bufferView._byteSize;
	}
	else if (usage == SRenderInstance::EUsage::DYNAMIC)
	{
		_dynamicOffset = offset;
		SBufferView bufferView = _dynamicInstances->GetData();
		memcpy(&destination[offset], bufferView._elements, bufferView._byteSize);
		nextOffset = offset + bufferView._byteSize;
	}

	return nextOffset;
}