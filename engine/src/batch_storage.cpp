// batch_storage.cpp

#include "batch_storage.hpp"
#include "context.hpp"
#include "dynamic_array.hpp"
#include "render_batch.hpp"
#include "handle_allocator.hpp"
#include "instance_buffer.hpp"

triton::XBatchStorage::XBatchStorage(cContext* context) : iObject(context)
{
	_batches = _context->Create<XHandleAllocator<SRenderBatchSlot, HBatch, XLinearArray<XRenderBatch>, XRenderBatch>>(_context);
	_batches->Initialize();
}

triton::XBatchStorage::~XBatchStorage()
{
	_context->Destroy<XHandleAllocator<SRenderBatchSlot, HBatch, XLinearArray<XRenderBatch>, XRenderBatch>>(_batches);
}

std::optional<triton::HBatch> triton::XBatchStorage::Create(const SGeometryView& geometry)
{
	return _batches->Create(_context, geometry);
}

triton::XRenderBatch* triton::XBatchStorage::Get(const HBatch& batch)
{
	return _batches->Get(batch);
}

void triton::XBatchStorage::Remove(const HBatch& batch)
{
	_batches->Destroy(batch);
}

std::optional<triton::SBatchInstance> triton::XBatchStorage::AddInstance(const HBatch& batch, SRenderInstance::EUsage usage)
{
	XRenderBatch* currentBatch = _batches->Get(batch);
        SRenderInstance ri = {};

	return currentBatch->Add(batch, usage, ri);
}

void triton::XBatchStorage::RemoveInstance(const SBatchInstance& instance)
{
	XRenderBatch* currentBatch = _batches->Get(instance.batch);
	currentBatch->Remove(instance);
}
