// batcher.cpp

#include "batcher.hpp"
#include "context.hpp"
#include "dynamic_array.hpp"
#include "handle_allocator.hpp"
#include "batch_data.hpp"
#include "game_object_subsystem.hpp"
#include "vertex.hpp"
#include "synchronization.hpp"

using namespace types;

triton::XBatchSubsystem::XBatchSubsystem(cContext* context) : ISubsys(context)
{
	_batchPool = CObjectAllocator::Create<XBatchPool>(
		64,
		_context,
		K_TRUE
	);
	_staticInstancePool = CObjectAllocator::Create<XStaticRenderInstancePool>(
		64,
		_context,
		K_TRUE,
		(s32)0,
		cBuffer::eType::STORAGE
	);
	_dynamicInstancePool = CObjectAllocator::Create<XDynamicRenderInstancePool>(
		64,
		_context,
		K_TRUE,
		(s32)1,
		cBuffer::eType::STORAGE
	);

	const sCapabilities* caps = _context->GetSubsystem<cEngine>()->GetCapabilities();
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

	CObjectAllocator::Destroy<XDynamicRenderInstancePool>(_dynamicInstancePool);
	CObjectAllocator::Destroy<XStaticRenderInstancePool>(_staticInstancePool);
	CObjectAllocator::Destroy<XBatchPool>(_batchPool);
}

std::optional<triton::SBatchData::THandle> triton::XBatchSubsystem::Create(
	ERenderInstanceMotionType motionType,
	const SGeometryView& geometry
)
{
	SBatchData bd;
	bd.bufferOffset = 0;
	bd.instanceCount = 0;
	bd.motionType = motionType;
	bd.sharedGeometry = geometry;
	
	return _batchPool->Create(std::move(bd));
}

triton::SBatchData& triton::XBatchSubsystem::Get(const SBatchData::THandle& batch)
{
	return *_batchPool->Get(batch);
}

void triton::XBatchSubsystem::Destroy(const SBatchData::THandle& batch)
{
	_batchPool->Destroy(batch);
}

std::optional<triton::SStaticRenderInstanceData::THandle> triton::XBatchSubsystem::AddStaticInstance(
	const SBatchData::THandle& batch,
	const SGameObjectData::THandle& gameObject
)
{
	auto riResult = _staticInstancePool->Create();
	if (!riResult.has_value())
		return std::nullopt;
	auto ri = *riResult;

	auto rdResult = _staticInstancePool->Get(ri);
	if (!rdResult.has_value())
		return std::nullopt;
	auto rd = *rdResult;

	rd.get().batch = batch;

	auto bdResult = _batchPool->Get(batch);
	if (!bdResult.has_value())
		return std::nullopt;
	auto bd = *bdResult;

	bd.get().instanceCount += 1;

	MarkDirtyStatic();

	return ri;
}

std::optional<triton::SDynamicRenderInstanceData::THandle> triton::XBatchSubsystem::AddDynamicInstance(
	const SBatchData::THandle& batch,
	const SGameObjectData::THandle& gameObject
)
{
	auto riResult = _dynamicInstancePool->Create();
	if (!riResult.has_value())
		return std::nullopt;
	auto ri = *riResult;

	auto rdResult = _dynamicInstancePool->Get(ri);
	if (!rdResult.has_value())
		return std::nullopt;
	auto rd = *rdResult;

	rd.get().batch = batch;

	auto bdResult = _batchPool->Get(batch);
	if (!bdResult.has_value())
		return std::nullopt;
	auto bd = *bdResult;

	bd.get().instanceCount += 1;

	MarkDirtyDynamic();

	return ri;
}

void triton::XBatchSubsystem::RemoveStaticInstance(const SStaticRenderInstanceData::THandle& instance)
{
	auto sriResult = _staticInstancePool->Get(instance);
	if (!sriResult.has_value())
		return;
	auto sri = *sriResult;

	auto bdResult = _batchPool->Get(sri.get().batch);
	if (!bdResult.has_value())
		return;
	auto bd = *bdResult;
	bd.get().instanceCount -= 1;

	_staticInstancePool->Destroy(instance);

	MarkDirtyStatic();
}

void triton::XBatchSubsystem::RemoveDynamicInstance(const SDynamicRenderInstanceData::THandle& instance)
{
	auto driResult = _dynamicInstancePool->Get(instance);
	if (!driResult.has_value())
		return;
	auto dri = *driResult;

	auto bdResult = _batchPool->Get(dri.get().batch);
	if (!bdResult.has_value())
		return;
	auto bd = *bdResult;
	bd.get().instanceCount -= 1;

	_dynamicInstancePool->Destroy(instance);

	MarkDirtyDynamic();
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

	_staticInstancePool->Update();
	_dynamicInstancePool->Update();
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
	const SBufferView<SBatchData> bvBatch = _batchPool->GetData();
	usize cumInstanceCount[2] = {};
	for (usize i = 0; i < bvBatch.elementCount; i++)
	{
		const usize batchIdx =
			*_batchPool->GetBufferIndex(*_batchPool->GetHandle(i));
		
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
		_staticInstancePool->GetData();
	for (usize instanceIdx = 0; instanceIdx < bvStaticInstances.elementCount; instanceIdx++)
	{
		SStaticRenderInstanceData& rid = bvStaticInstances.elements[instanceIdx];
		SBatchData& bd = *_batchPool->Get(rid.batch);

		if (bd.motionType != motionType)
			continue;

		const usize batchIdx = *_batchPool->GetBufferIndex(rid.batch);
		const usize globElementIndex =
			bd.bufferOffset +
			counterBuffer[batchIdx];
		counterBuffer[batchIdx] += 1;

		_staticInstancePool->WriteToStaging(
			globElementIndex,
			rid
		);
	}

	// Dynamic
	const SBufferView<SDynamicRenderInstanceData> bvDynamicInstances =
		_dynamicInstancePool->GetData();
	for (usize instanceIdx = 0; instanceIdx < bvDynamicInstances.elementCount; instanceIdx++)
	{
		SDynamicRenderInstanceData& rid = bvDynamicInstances.elements[instanceIdx];
		SBatchData& bd = *_batchPool->Get(rid.batch);

		if (bd.motionType != motionType)
			continue;

		const usize batchIdx = *_batchPool->GetBufferIndex(rid.batch);
		const usize globElementIndex =
			bd.bufferOffset +
			counterBuffer[batchIdx];
		counterBuffer[batchIdx] += 1;

		_dynamicInstancePool->WriteToStaging(
			globElementIndex,
			rid
		);
	}
}