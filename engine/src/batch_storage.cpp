// batch_storage.cpp

#include "batch_storage.hpp"
#include "context.hpp"
#include "stack.hpp"
#include "render_batch.hpp"
#include "handle_allocator.hpp"

triton::XBatchStorage::XBatchStorage(cContext* context) : iObject(context)
{
	_batches = _context->Create<XHandleAllocator<SRenderBatchSlot, SRenderBatchHandle, cStack<XRenderBatch>, XRenderBatch>>(_context);
}

triton::XBatchStorage::~XBatchStorage()
{
	_context->Destroy<XHandleAllocator<SRenderBatchSlot, SRenderBatchHandle, cStack<XRenderBatch>, XRenderBatch>>(_batches);
}

std::optional<triton::SRenderBatchHandle> triton::XBatchStorage::Create(const SGeometryView& geometry)
{
	return _batches->Create(_context, geometry);
}

void triton::XBatchStorage::Remove(const SRenderBatchHandle& batch)
{
	_batches->Remove(batch);
}