// render_batch.cpp

#include "render_batch.hpp"
#include "context.hpp"
#include "instance_buffer.hpp"
#include "handle_allocator.hpp"

using namespace types;

triton::XRenderBatch::XRenderBatch(cContext* context, const SGeometryView& geometry) : iObject(context), _geometry(geometry)
{
	_staticInstances = _context->Create<XHandleAllocator<SInstanceBufferSlot, HRenderInstance, XLinearArray<SRenderInstance>, SRenderInstance>>(_context);
	_staticInstances->Initialize();
	_dynamicInstances = _context->Create<XHandleAllocator<SInstanceBufferSlot, HRenderInstance, XLinearArray<SRenderInstance>, SRenderInstance>>(_context);
	_dynamicInstances->Initialize();
}

triton::XRenderBatch::~XRenderBatch()
{
	_dynamicInstances->Free();
	_context->Destroy<XHandleAllocator<SInstanceBufferSlot, HRenderInstance, XLinearArray<SRenderInstance>, SRenderInstance>>(_dynamicInstances);
	_staticInstances->Free();
	_context->Destroy<XHandleAllocator<SInstanceBufferSlot, HRenderInstance, XLinearArray<SRenderInstance>, SRenderInstance>>(_staticInstances);
}

std::optional<triton::SBatchInstance> triton::XRenderBatch::Add(const HBatch& batch, SRenderInstance::EUsage usage, SRenderInstance& instance)
{
	HRenderInstance ri;
	if (usage == SRenderInstance::EUsage::STATIC)
		ri = _staticInstances->Create(std::move(instance));
	else if (usage == SRenderInstance::EUsage::DYNAMIC)
		ri = _dynamicInstances->Create(std::move(instance));

	SBatchInstance bi;
	bi.usage = usage;
	bi.batch = batch;
	bi.instance = ri;

	return bi;
}

triton::SRenderInstance* triton::XRenderBatch::Get(const SBatchInstance& handle)
{
	if (handle.usage == SRenderInstance::EUsage::STATIC)
		return _staticInstances->Get(handle.instance);
	else if (handle.usage == SRenderInstance::EUsage::DYNAMIC)
		return _dynamicInstances->Get(handle.instance);

	return nullptr;
}

void triton::XRenderBatch::Set(const SBatchInstance& handle, const SRenderInstance& instance)
{
	if (handle.usage == SRenderInstance::EUsage::STATIC)
		*_staticInstances->Get(handle.instance) = instance;
	else if (handle.usage == SRenderInstance::EUsage::DYNAMIC)
		*_dynamicInstances->Get(handle.instance) = instance;
}

void triton::XRenderBatch::Remove(const SBatchInstance& handle)
{
	if (handle.usage == SRenderInstance::EUsage::STATIC)
		return _staticInstances->Destroy(handle.instance);
	else if (handle.usage == SRenderInstance::EUsage::DYNAMIC)
		return _dynamicInstances->Destroy(handle.instance);
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
	else
	{
		return 0;
	}

	return nextOffset;
}