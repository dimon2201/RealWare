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

triton::XBatchSubsystem::XBatchSubsystem(cContext* context)
	:
	ISubsys(context),
	CUploader<SStaticRenderInstanceData, HStaticRenderInstance, XLinearArray<SStaticRenderInstanceData>, SGPUStaticRenderInstanceLayout>(
		context,
		(cGPUResource**)&_staticGPUBuffer,
		context->GetSubsystem<cEngine>()->GetCapabilities()->maxRenderStaticInstanceCount,
		K_FALSE
	),
	CUploader<SDynamicRenderInstanceData, HDynamicRenderInstance, XLinearArray<SDynamicRenderInstanceData>, SGPUDynamicRenderInstanceLayout>(
		context,
		(cGPUResource**)&_dynamicGPUBuffer,
		context->GetSubsystem<cEngine>()->GetCapabilities()->maxRenderDynamicInstanceCount,
		K_TRUE
	)
{
	_batches = _context->Create<CHandleAllocator<SSlot, SBatchData, HBatch, XLinearArray<SBatchData>>>(
		_context,
		4096,
		4096,
		65536 * 64,
		1024
	);
	
	const sCapabilities* caps = _context->GetSubsystem<cEngine>()->GetCapabilities();

    XRenderSubsystem* renderSubsystem = _context->GetSubsystem<XRenderSubsystem>();
    renderSubsystem->PushCommand(SRenderCommand(
        ERenderCommand::CREATE_BUFFER,
        (cpuword)cBuffer::eType::STORAGE,
        (cpuword)nullptr,
        caps->maxRenderStaticInstanceCount * sizeof(SGPUStaticRenderInstanceLayout),
        0
    ));
    _staticGPUBuffer = renderSubsystem->FetchResult<cBuffer*>();
    renderSubsystem->PushCommand(SRenderCommand(
        ERenderCommand::CREATE_BUFFER,
        (cpuword)cBuffer::eType::STORAGE,
        (cpuword)nullptr,
        caps->maxRenderDynamicInstanceCount * sizeof(SGPUDynamicRenderInstanceLayout),
        1
    ));
    _dynamicGPUBuffer = renderSubsystem->FetchResult<cBuffer*>();

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

	XRenderSubsystem* renderSubsystem = _context->GetSubsystem<XRenderSubsystem>();
	renderSubsystem->PushCommand(SRenderCommand(
		ERenderCommand::DESTROY_BUFFER,
		(cpuword)_dynamicGPUBuffer
	));
	renderSubsystem->PushCommand(SRenderCommand(
		ERenderCommand::DESTROY_BUFFER,
		(cpuword)_staticGPUBuffer
	));

	_context->Destroy<CHandleAllocator<SSlot, SBatchData, HBatch, XLinearArray<SBatchData>>>(_batches);
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

triton::HStaticRenderInstance triton::XBatchSubsystem::AddStaticInstance(
	const HBatch& batch,
	const HGameObject& gameObject
)
{
	HStaticRenderInstance ri = CUploader<
		SStaticRenderInstanceData,
		HStaticRenderInstance,
		XLinearArray<SStaticRenderInstanceData>,
		SGPUStaticRenderInstanceLayout
	>::Create();

	SStaticRenderInstanceData& rid = CUploader<
		SStaticRenderInstanceData,
		HStaticRenderInstance,
		XLinearArray<SStaticRenderInstanceData>,
		SGPUStaticRenderInstanceLayout
	>::Get(ri);
	rid.batch = batch;
	rid.gameObject = gameObject;

	SBatchData& bd = _batches->Get(batch);
	bd.instanceCount += 1;

	MarkDirtyStatic();

	return ri;
}

triton::HDynamicRenderInstance triton::XBatchSubsystem::AddDynamicInstance(
	const HBatch& batch,
	const HGameObject& gameObject
)
{
	HDynamicRenderInstance ri = CUploader<
		SDynamicRenderInstanceData,
		HDynamicRenderInstance,
		XLinearArray<SDynamicRenderInstanceData>,
		SGPUDynamicRenderInstanceLayout
	>::Create();

	SDynamicRenderInstanceData& rid = CUploader<
		SDynamicRenderInstanceData,
		HDynamicRenderInstance,
		XLinearArray<SDynamicRenderInstanceData>,
		SGPUDynamicRenderInstanceLayout
	>::Get(ri);
	rid.batch = batch;
	rid.gameObject = gameObject;

	SBatchData& bd = _batches->Get(batch);
	bd.instanceCount += 1;

	MarkDirtyDynamic();

	return ri;
}

void triton::XBatchSubsystem::RemoveStaticInstance(const HStaticRenderInstance& instance)
{
	SStaticRenderInstanceData sri = CUploader<
		SStaticRenderInstanceData,
		HStaticRenderInstance,
		XLinearArray<SStaticRenderInstanceData>,
		SGPUStaticRenderInstanceLayout
	>::Get(instance);

	SBatchData& bd = _batches->Get(sri.batch);
	bd.instanceCount -= 1;

	CUploader<
		SStaticRenderInstanceData,
		HStaticRenderInstance,
		XLinearArray<SStaticRenderInstanceData>,
		SGPUStaticRenderInstanceLayout
	>::Destroy(instance);

	MarkDirtyStatic();
}

void triton::XBatchSubsystem::RemoveDynamicInstance(const HDynamicRenderInstance& instance)
{
	SDynamicRenderInstanceData& rid = CUploader<
		SDynamicRenderInstanceData,
		HDynamicRenderInstance,
		XLinearArray<SDynamicRenderInstanceData>,
		SGPUDynamicRenderInstanceLayout>::Get(instance);

	SBatchData& bd = _batches->Get(rid.batch);
	bd.instanceCount -= 1;

	CUploader<
		SDynamicRenderInstanceData,
		HDynamicRenderInstance,
		XLinearArray<SDynamicRenderInstanceData>,
		SGPUDynamicRenderInstanceLayout
	>::Destroy(instance);

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
		PackInstancesToStagingBuffers(
			ERenderInstanceMotionType::Static,
			_tempStaticCounterBuffer
		);

		_bStaticBufferNeedsPacking = K_FALSE;
	}
	if (DynamicBufferNeedsPacking())
	{
		RecalcBufferOffsetsAndResetCounters(_tempDynamicCounterBuffer);
		PackInstancesToStagingBuffers(
			ERenderInstanceMotionType::Dynamic,
			_tempDynamicCounterBuffer
		);

		_bDynamicBufferNeedsPacking = K_FALSE;
	}

	CUploader<
		SStaticRenderInstanceData,
		HStaticRenderInstance,
		XLinearArray<SStaticRenderInstanceData>,
		SGPUStaticRenderInstanceLayout
	>::UploadStagingToGpuIfDirty(_context->GetSubsystem<XRenderSubsystem>());
	CUploader<
		SDynamicRenderInstanceData,
		HDynamicRenderInstance,
		XLinearArray<SDynamicRenderInstanceData>,
		SGPUDynamicRenderInstanceLayout
	>::UploadStagingToGpuIfDirty(_context->GetSubsystem<XRenderSubsystem>());
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

void triton::XBatchSubsystem::PackInstancesToStagingBuffers(
	ERenderInstanceMotionType motionType,
	types::u32* counterBuffer
)
{
	// Static
	const SBufferView<SStaticRenderInstanceData> bvStaticInstances =
		CUploader<
			SStaticRenderInstanceData,
			HStaticRenderInstance,
			XLinearArray<SStaticRenderInstanceData>,
			SGPUStaticRenderInstanceLayout
		>::GetData();
	for (usize instanceIdx = 0; instanceIdx < bvStaticInstances.elementCount; instanceIdx++)
	{
		SStaticRenderInstanceData& rid = bvStaticInstances.elements[instanceIdx];
		SBatchData& bd = _batches->Get(rid.batch);

		if (bd.motionType != motionType)
			continue;

		SGPUStaticRenderInstanceLayout gpuElementData;
		gpuElementData._use2D = 0.0f;
		gpuElementData._world = rid.worldMatrix;
		gpuElementData._skeletonIndex =
			_context->GetSubsystem<XSkeletonSubsystem>()->GetHandleBufferIndex(rid.skeleton);
		gpuElementData._materialIndex =
			_context->GetSubsystem<XMaterialSubsystem>()->GetHandleBufferIndex(rid.material);

		const usize batchIdx = _batches->GetHandleBufferIndex(rid.batch);
		const usize globElementIndex =
			bd.bufferOffset +
			counterBuffer[batchIdx];
		counterBuffer[batchIdx] += 1;

		CUploader<
			SStaticRenderInstanceData,
			HStaticRenderInstance,
			XLinearArray<SStaticRenderInstanceData>,
			SGPUStaticRenderInstanceLayout
		>::WriteToStaging(
			globElementIndex,
			&gpuElementData,
			1
		);
	}

	// Dynamic
	const SBufferView<SDynamicRenderInstanceData> bvDynamicInstances =
		CUploader<
			SDynamicRenderInstanceData,
			HDynamicRenderInstance,
			XLinearArray<SDynamicRenderInstanceData>,
			SGPUDynamicRenderInstanceLayout
		>::GetData();
	for (usize instanceIdx = 0; instanceIdx < bvDynamicInstances.elementCount; instanceIdx++)
	{
		SDynamicRenderInstanceData& rid = bvDynamicInstances.elements[instanceIdx];
		SBatchData& bd = _batches->Get(rid.batch);

		if (bd.motionType != motionType)
			continue;

		SGPUDynamicRenderInstanceLayout gpuElementData;
		gpuElementData._use2D = 0.0f;
		gpuElementData._world = rid.worldMatrix;
		gpuElementData._skeletonIndex =
			_context->GetSubsystem<XSkeletonSubsystem>()->GetHandleBufferIndex(rid.skeleton);
		gpuElementData._materialIndex =
			_context->GetSubsystem<XMaterialSubsystem>()->GetHandleBufferIndex(rid.material);

		const usize batchIdx = _batches->GetHandleBufferIndex(rid.batch);
		const usize globElementIndex =
			bd.bufferOffset +
			counterBuffer[batchIdx];
		counterBuffer[batchIdx] += 1;

		CUploader<
			SDynamicRenderInstanceData,
			HDynamicRenderInstance,
			XLinearArray<SDynamicRenderInstanceData>,
			SGPUDynamicRenderInstanceLayout
		>::WriteToStaging(
			globElementIndex,
			&gpuElementData,
			1
		);
	}
}