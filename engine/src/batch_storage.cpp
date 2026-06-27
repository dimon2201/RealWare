// batch_storage.cpp

#include "batch_storage.hpp"
#include "context.hpp"
#include "stack.hpp"
#include "render_batch.hpp"
#include "handle_allocator.hpp"
#include "instance_buffer.hpp"

triton::XBatchStorage::XBatchStorage(cContext* context) : iObject(context)
{
	_batches = _context->Create<XHandleAllocator<SRenderBatchSlot, SRenderBatchHandle, XLinearArray<XRenderBatch>, XRenderBatch>>(_context);
	_batches->Initialize();
}

triton::XBatchStorage::~XBatchStorage()
{
	_context->Destroy<XHandleAllocator<SRenderBatchSlot, SRenderBatchHandle, XLinearArray<XRenderBatch>, XRenderBatch>>(_batches);
}

std::optional<triton::SRenderBatchHandle> triton::XBatchStorage::Create(const SGeometryView& geometry)
{
	return _batches->Create(SRenderBatchHandle(), _context, geometry);
}

void triton::XBatchStorage::Remove(const SRenderBatchHandle& batch)
{
	_batches->Destroy(batch);
}

std::optional<triton::SInstanceBufferHandle> triton::XBatchStorage::AddInstance(const SRenderBatchHandle& batch, SRenderInstance::EUsage usage)
{
	XRenderBatch* currentBatch = _batches->Get(batch);

	return currentBatch->Add(batch, usage, SRenderInstance());
}

void triton::XBatchStorage::RemoveInstance(const SInstanceBufferHandle& instance)
{
	XRenderBatch* currentBatch = _batches->Get(instance._batch);
	currentBatch->Remove(instance);
}