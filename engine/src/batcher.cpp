// batcher.cpp

#include "batcher.hpp"
#include "context.hpp"
#include "dynamic_array.hpp"
#include "handle_allocator.hpp"
#include "batch_data.hpp"
#include "render_instance.hpp"
#include "game_object_subsystem.hpp"
#include "vertex.hpp"

using namespace types;

triton::XBatchSubsystem::XBatchSubsystem(cContext* context) : iObject(context)
{
	_batches = _context->Create<XHandleAllocator<SSlot, SBatchData, HBatch, XLinearArray<SBatchData>>>(_context);
	
	const sCapabilities* caps = _context->GetSubsystem<cEngine>()->GetCapabilities();

    XRenderSubsystem* renderSubsystem = _context->GetSubsystem<XRenderSubsystem>();
    renderSubsystem->PushCommand(SRenderCommand(
        ERenderCommand::CREATE_BUFFER,
        (cpuword)cBuffer::eType::STORAGE,
        (cpuword)nullptr,
        caps->maxRenderInstanceCount * sizeof(SGPURenderInstanceLayout),
        0
    ));
    _staticGPUBuffer = renderSubsystem->FetchResult<cBuffer*>();
    renderSubsystem->PushCommand(SRenderCommand(
        ERenderCommand::CREATE_BUFFER,
        (cpuword)cBuffer::eType::STORAGE,
        (cpuword)nullptr,
        caps->maxRenderInstanceCount * sizeof(SGPURenderInstanceLayout),
        1
    ));
    _dynamicGPUBuffer = renderSubsystem->FetchResult<cBuffer*>();

	_staticStorage = _context->Create<XStaticInstanceStorage>(
		_context,
		_staticGPUBuffer,
		caps->maxRenderInstanceCount,
		K_FALSE
	);
	_dynamicStorage = _context->Create<XDynamicInstanceStorage>(
		_context,
		_dynamicGPUBuffer,
		caps->maxRenderInstanceCount,
		K_TRUE
	);

	_tempStaticCounterBuffer = (u32*)_context->GetMemoryAllocator()->Allocate(
		caps->maxRenderBatchCount * sizeof(u32),
		64
	);
	_tempDynamicCounterBuffer = (u32*)_context->GetMemoryAllocator()->Allocate(
		caps->maxRenderBatchCount * sizeof(u32),
		64
	);
}

triton::XBatchSubsystem::~XBatchSubsystem()
{
	_context->GetMemoryAllocator()->Deallocate(_tempDynamicCounterBuffer);
	_context->GetMemoryAllocator()->Deallocate(_tempStaticCounterBuffer);

	_context->Destroy<XDynamicInstanceStorage>(_dynamicStorage);
	_context->Destroy<XStaticInstanceStorage>(_staticStorage);

	XRenderSubsystem* renderSubsystem = _context->GetSubsystem<XRenderSubsystem>();
	renderSubsystem->PushCommand(SRenderCommand(
		ERenderCommand::DESTROY_BUFFER,
		(cpuword)_dynamicGPUBuffer
	));
	renderSubsystem->PushCommand(SRenderCommand(
		ERenderCommand::DESTROY_BUFFER,
		(cpuword)_staticGPUBuffer
	));

	_context->Destroy<XHandleAllocator<SSlot, SBatchData, HBatch, XLinearArray<SBatchData>>>(_batches);
}

std::optional<triton::HBatch> triton::XBatchSubsystem::Create(
	ERenderInstanceMotionType motionType,
	const SGeometryView& geometry
)
{
	SBatchData bd;
	bd.bufferOffset = 0;
	bd.instanceCount = 0;
	bd.motionType = motionType;
	bd.sharedGeometry = geometry;
	
	return _batches->Create(std::move(bd));
}

triton::SBatchData& triton::XBatchSubsystem::Get(const HBatch& batch)
{
	return _batches->Get(batch);
}

void triton::XBatchSubsystem::Destroy(const HBatch& batch)
{
	_batches->Destroy(batch);
}

triton::HRenderInstance triton::XBatchSubsystem::AddStaticInstance(
	const HBatch& batch,
	const HGameObject& gameObject
)
{
	HRenderInstance ri = _staticStorage->CreateStaticInstance(batch);
	SRenderInstanceData& rid = _staticStorage->Get(ri);
	rid.batch = batch;
	rid.gameObject = gameObject;
	SBatchData& bd = _batches->Get(batch);
	bd.instanceCount += 1;

	MarkDirtyStatic();

	return ri;
}

triton::HRenderInstance triton::XBatchSubsystem::AddDynamicInstance(
	const HBatch& batch,
	const HGameObject& gameObject
)
{
	HRenderInstance ri = _dynamicStorage->CreateDynamicInstance(batch);
	SRenderInstanceData& rid = _dynamicStorage->Get(ri);
	rid.batch = batch;
	rid.gameObject = gameObject;
	SBatchData& bd = _batches->Get(batch);
	bd.instanceCount += 1;

	MarkDirtyDynamic();

	return ri;
}

void triton::XBatchSubsystem::RemoveStaticInstance(const HRenderInstance& instance)
{
	SRenderInstanceData sri = _staticStorage->Get(instance);
	SBatchData& bd = _batches->Get(sri.batch);
	bd.instanceCount -= 1;

	_staticStorage->DestroyStaticInstance(instance);

	MarkDirtyStatic();
}

void triton::XBatchSubsystem::RemoveDynamicInstance(const HRenderInstance& instance)
{
	SRenderInstanceData& rid = _dynamicStorage->Get(instance);
	SBatchData& bd = _batches->Get(rid.batch);
	bd.instanceCount -= 1;

	_dynamicStorage->DestroyDynamicInstance(instance);

	MarkDirtyDynamic();
}

std::optional<triton::HBatch> triton::XBatchSubsystem::FindSimilarBatch(
	const types::u8* vertexBytes,
	types::usize vertexBytesCount,
	const types::u8* indexBytes,
	types::usize indexBytesCount
)
{
	SBufferView<SBatchData> bv = _batches->GetData();
	for (usize i = 0; i < bv.elementCount; i++)
	{
		const SBatchData& batch = bv.elements[i];
		if (batch.sharedGeometry._vertexData == vertexBytes &&
			batch.sharedGeometry._vertexCount * sizeof(SVertex) == vertexBytesCount &&
			batch.sharedGeometry._indexData == indexBytes &&
			batch.sharedGeometry._indexCount * sizeof(u32) == indexBytesCount)
			return _batches->GetHandle(i);
	}

	return std::nullopt;
}

void triton::XBatchSubsystem::Update()
{
	if (StaticBufferNeedsPacking())
	{
		RecalcBufferOffsetsAndResetCounters(_tempStaticCounterBuffer);
		PackInstancesToStagingBuffer(
			ERenderInstanceMotionType::Static,
			_tempStaticCounterBuffer,
			_staticStorage
		);

		_bStaticBufferNeedsPacking = K_FALSE;
	}
	if (DynamicBufferNeedsPacking())
	{
		RecalcBufferOffsetsAndResetCounters(_tempDynamicCounterBuffer);
		PackInstancesToStagingBuffer(
			ERenderInstanceMotionType::Dynamic,
			_tempDynamicCounterBuffer,
			_dynamicStorage
		);

		_bDynamicBufferNeedsPacking = K_FALSE;
	}

	_staticStorage->UploadStagingToGpuIfDirty();
	_dynamicStorage->UploadStagingToGpuIfDirty();
}

void triton::XBatchSubsystem::MarkDirtyStatic()
{
	_bStaticBufferNeedsPacking = K_TRUE;
}

void triton::XBatchSubsystem::MarkDirtyDynamic()
{
	_bDynamicBufferNeedsPacking = K_TRUE;
}

types::boolean triton::XBatchSubsystem::StaticBufferNeedsPacking()
{
	return _bStaticBufferNeedsPacking == K_TRUE;
}

types::boolean triton::XBatchSubsystem::DynamicBufferNeedsPacking()
{
	return _bDynamicBufferNeedsPacking == K_TRUE;
}

void triton::XBatchSubsystem::RecalcBufferOffsetsAndResetCounters(u32* counterBuffer)
{
	const SBufferView<SBatchData> bvBatch = _batches->GetData();
	usize cumInstanceCount[2] = {};
	for (usize i = 0; i < bvBatch.elementCount; i++)
	{
		const usize batchIdx =
			_batches->GetHandleBufferIndex(_batches->GetHandle(i));
		
		SBatchData& bd = bvBatch.elements[batchIdx];
		bd.bufferOffset = cumInstanceCount[(int)bd.motionType];
		cumInstanceCount[(int)bd.motionType] += bd.instanceCount;

		counterBuffer[batchIdx] = 0;
	}
}