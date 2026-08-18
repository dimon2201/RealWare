// batcher.cpp

#include "batcher.hpp"
#include "context.hpp"
#include "dynamic_array.hpp"
#include "handle_allocator.hpp"
#include "batch_data.hpp"
#include "game_object_subsystem.hpp"
#include "vertex.hpp"
#include "synchronization.hpp"
#include "skin_pool.hpp"

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
		EGPUBufferType::Storage
	);
	_dynamicInstancePool = CObjectAllocator::Create<XDynamicRenderInstancePool>(
		64,
		_context,
		K_TRUE,
		(s32)1,
		EGPUBufferType::Storage
	);
}

triton::XBatchSubsystem::~XBatchSubsystem()
{
	CObjectAllocator::Destroy<XDynamicRenderInstancePool>(_dynamicInstancePool);
	CObjectAllocator::Destroy<XStaticRenderInstancePool>(_staticInstancePool);
	CObjectAllocator::Destroy<XBatchPool>(_batchPool);
}

std::optional<triton::SBatchData::THandle> triton::XBatchSubsystem::Create(
	ERenderInstanceMotionType motionType,
	const SGeometryView& geometry,
	types::usize maxReservedInstanceCount
)
{
	SBatchData bd;
	bd.bufferOffset = 0;
	bd.lastCreatedInstanceCursor = 0;
	bd.instanceCount = 0;
	bd.motionType = motionType;
	bd.sharedGeometry = geometry;
	
	if (motionType == ERenderInstanceMotionType::Static)
		bd.staticsFrame = *_staticInstancePool->Create(maxReservedInstanceCount);
	else if (motionType == ERenderInstanceMotionType::Dynamic)
		bd.dynamicsFrame = *_dynamicInstancePool->Create(maxReservedInstanceCount);
	
	bd.maxReservedInstanceCount = maxReservedInstanceCount;

	return _batchPool->Create(std::move(bd));
}

triton::SBatchData& triton::XBatchSubsystem::Get(const SBatchData::THandle& batch)
{
	return *_batchPool->Get(batch);
}

void triton::XBatchSubsystem::Destroy(const SBatchData::THandle& batch)
{
	SBatchData& data = *_batchPool->Get(batch);
	if (data.motionType == ERenderInstanceMotionType::Static)
		_staticInstancePool->Destroy(data.staticsFrame);
	else if (data.motionType == ERenderInstanceMotionType::Dynamic)
		_dynamicInstancePool->Destroy(data.dynamicsFrame);

	_batchPool->Destroy(batch);
}

std::optional<triton::SStaticRenderInstanceData::THandle> triton::XBatchSubsystem::AddStaticInstance(
	const SBatchData::THandle& batch
)
{
	SBatchData& data = *_batchPool->Get(batch);

	if (data.instanceCount >= data.maxReservedInstanceCount)
		return std::nullopt;

	data.instanceCount += 1;

	s32 instanceIndex = -1;
	if (data.freeFrameIndices.empty())
	{
		instanceIndex = data.lastCreatedInstanceCursor++;
		if (data.lastCreatedInstanceCursor > data.maxReservedInstanceCount)
			return std::nullopt;
	}
	else
	{
		instanceIndex = data.freeFrameIndices.back();
		data.freeFrameIndices.pop_back();
	}

	const SStaticRenderInstanceData::THandle& beginHandle = data.staticsFrame.begin;
	const SStaticRenderInstanceData::THandle curHandle = *_staticInstancePool->GetHandle(beginHandle, instanceIndex);
	SStaticRenderInstanceData& srid = *_staticInstancePool->Get(curHandle);
	srid.batchInstanceIndex = instanceIndex;

	MarkDirtyStatic();

	return curHandle;
}

std::optional<triton::SDynamicRenderInstanceData::THandle> triton::XBatchSubsystem::AddDynamicInstance(
	const SBatchData::THandle& batch
)
{
	SBatchData& data = *_batchPool->Get(batch);

	data.instanceCount += 1;

	s32 instanceIndex = -1;
	if (data.freeFrameIndices.empty())
	{
		instanceIndex = data.lastCreatedInstanceCursor++;
		if (data.lastCreatedInstanceCursor > data.maxReservedInstanceCount)
			return std::nullopt;
	}
	else
	{
		instanceIndex = data.freeFrameIndices.back();
		data.freeFrameIndices.pop_back();
	}

	const SDynamicRenderInstanceData::THandle& beginHandle = data.dynamicsFrame.begin;
	const SDynamicRenderInstanceData::THandle curHandle = *_dynamicInstancePool->GetHandle(beginHandle, instanceIndex);
	SDynamicRenderInstanceData& drid = *_dynamicInstancePool->Get(curHandle);
	drid.batchInstanceIndex = instanceIndex;

	MarkDirtyDynamic();

	return curHandle;
}

void triton::XBatchSubsystem::SetStaticInstanceWorldMatrix(
	const SStaticRenderInstanceData::THandle& instance,
	const cMatrix4& matrix
)
{
	SStaticRenderInstanceData& srid = *_staticInstancePool->Get(instance);
	srid.worldMatrix = matrix;

	MarkDirtyStatic();
}

void triton::XBatchSubsystem::SetStaticInstanceMaterial(
	const SStaticRenderInstanceData::THandle& instance,
	const SMaterialData::THandle& material
)
{
	SStaticRenderInstanceData& srid = *_staticInstancePool->Get(instance);
	srid.material = material;

	MarkDirtyStatic();
}

void triton::XBatchSubsystem::SetStaticInstanceSkin(
	const SStaticRenderInstanceData::THandle& instance,
	const SSkinData::THandle& skin
)
{
	const SSkinData& skinData = *_context->GetSubsystem<XSkinningSubsystem>()->GetSkinPool()->Get(skin);

	SStaticRenderInstanceData& srid = *_staticInstancePool->Get(instance);
	srid.skinnedBoneBufferOffset = skinData.globSkinnedBoneBufferOffset;

	MarkDirtyStatic();
}

void triton::XBatchSubsystem::SetDynamicInstanceWorldMatrix(
	const SDynamicRenderInstanceData::THandle& instance,
	const cMatrix4& matrix
)
{
	SDynamicRenderInstanceData& drid = *_dynamicInstancePool->Get(instance);
	drid.worldMatrix = matrix;

	MarkDirtyDynamic();
}

void triton::XBatchSubsystem::RemoveStaticInstance(
	const SBatchData::THandle& batch,
	const SStaticRenderInstanceData::THandle& instance
)
{
	SStaticRenderInstanceData& srid = *_staticInstancePool->Get(instance);

	SBatchData& bd = *_batchPool->Get(batch);
	bd.instanceCount -= 1;
	bd.freeFrameIndices.push_back(srid.batchInstanceIndex);

	MarkDirtyStatic();
}

void triton::XBatchSubsystem::RemoveDynamicInstance(
	const SBatchData::THandle& batch, 
	const SDynamicRenderInstanceData::THandle& instance
)
{
	SDynamicRenderInstanceData& drid = *_dynamicInstancePool->Get(instance);

	SBatchData& bd = *_batchPool->Get(batch);
	bd.instanceCount -= 1;
	bd.freeFrameIndices.push_back(drid.batchInstanceIndex);

	MarkDirtyDynamic();
}

void triton::XBatchSubsystem::Update()
{
	if (_bStaticDirtyBit == K_TRUE)
		PackStaticInstancesToStagingBuffer();
	if (_bDynamicDirtyBit == K_TRUE)
		PackDynamicInstancesToStagingBuffer();

	_staticInstancePool->Update();
	_dynamicInstancePool->Update();
}

void triton::XBatchSubsystem::MarkDirtyStatic()
{
	_bStaticDirtyBit = K_TRUE;
}

void triton::XBatchSubsystem::MarkDirtyDynamic()
{
	_bDynamicDirtyBit = K_TRUE;
}

void triton::XBatchSubsystem::PackStaticInstancesToStagingBuffer()
{
	SBufferView<SBatchData> batches = _batchPool->GetData();
	for (usize i = 0; i < batches.elementCount; i++)
	{
		SBatchData& batch = batches.elements[i];
		const SObjectFrame<SStaticRenderInstanceData::THandle> frame = batch.staticsFrame;
		
		const usize beginObjectIndex = _staticInstancePool->GetPackedIndex(frame.begin);
		const usize objectCount = frame.count;
		usize aliveObjectCount = 0;

		for (usize j = 0; j < objectCount; j++)
		{
			boolean bIsDeleted = False;
			for (usize k = 0; k < batch.freeFrameIndices.size(); k++)
			{
				if (j == batch.freeFrameIndices[k])
				{
					bIsDeleted = True;
					break;
				}
			}

			if (bIsDeleted == False)
			{
				const SStaticRenderInstanceData::THandle handle = *_staticInstancePool->GetHandle(frame.begin, j);
				SStaticRenderInstanceData& data = *_staticInstancePool->Get(handle);

				_staticInstancePool->WriteToStaging(
					beginObjectIndex + aliveObjectCount,
					data
				);

				++aliveObjectCount;
			}
		}

		batches.elements[i].bufferOffset = beginObjectIndex;
	}

	_bStaticDirtyBit = False;
}

void triton::XBatchSubsystem::PackDynamicInstancesToStagingBuffer()
{
	SBufferView<SBatchData> batches = _batchPool->GetData();
	for (usize i = 0; i < batches.elementCount; i++)
	{
		SBatchData& batch = batches.elements[i];
		const SObjectFrame<SDynamicRenderInstanceData::THandle> frame = batch.dynamicsFrame;

		const usize beginObjectIndex = _dynamicInstancePool->GetPackedIndex(frame.begin);
		const usize objectCount = frame.count;
		usize aliveObjectCount = 0;

		for (usize j = 0; j < objectCount; j++)
		{
			boolean bIsDeleted = False;
			for (usize k = 0; k < batch.freeFrameIndices.size(); k++)
			{
				if (j == batch.freeFrameIndices[k])
				{
					bIsDeleted = True;
					break;
				}
			}

			if (bIsDeleted == False)
			{
				const SDynamicRenderInstanceData::THandle handle = *_dynamicInstancePool->GetHandle(frame.begin, j);
				SDynamicRenderInstanceData& data = *_dynamicInstancePool->Get(handle);

				_dynamicInstancePool->WriteToStaging(
					beginObjectIndex + aliveObjectCount,
					data
				);

				++aliveObjectCount;
			}
		}

		batches.elements[i].bufferOffset = beginObjectIndex;
	}

	_bStaticDirtyBit = False;
}