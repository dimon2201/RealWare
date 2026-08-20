// render_instance_pack.cpp

#include "render_instance_pack.hpp"
#include "context.hpp"
#include "render_instance_static_pool.hpp"
#include "render_instance_dynamic_pool.hpp"
#include "render_instance.hpp"

using namespace types;

triton::XRenderInstancePack::XRenderInstancePack(
	cContext* context,
	s32 poolIndex,
	ERenderInstanceMotionType motionType,
	const SGeometryView& sharedGeometry,
	types::usize maxInstanceCount
) :
	iObject(context, poolIndex),
	_bufferOffset(0),
	_lastInstanceCursor(0),
	_instanceCount(0),
	_motionType(motionType),
	_sharedGeometry(sharedGeometry)
{
	CRenderInstancePool* renderInstancePool = nullptr;
	if (_motionType == ERenderInstanceMotionType::Static)
		renderInstancePool = _context->GetPool<CRenderInstanceStaticPool>();
	else if (_motionType == ERenderInstanceMotionType::Dynamic)
		renderInstancePool = _context->GetPool<CRenderInstanceDynamicPool>();

	_frame = *renderInstancePool->CreateFrame(maxInstanceCount);
	_maxInstanceCount = maxInstanceCount;
}

triton::XRenderInstancePack::~XRenderInstancePack()
{
	CRenderInstancePool* renderInstancePool = nullptr;
	if (_motionType == ERenderInstanceMotionType::Static)
		renderInstancePool = _context->GetPool<CRenderInstanceStaticPool>();
	else if (_motionType == ERenderInstanceMotionType::Dynamic)
		renderInstancePool = _context->GetPool<CRenderInstanceDynamicPool>();

	renderInstancePool->DestroyFrame(_frame);
}

std::optional<triton::XRenderInstance::THandle> triton::XRenderInstancePack::AddInstance()
{
	if (_instanceCount >= _maxInstanceCount)
		return std::nullopt;

	_instanceCount += 1;

	s32 instanceIndex = -1;
	if (_freeFrameIndices.empty())
	{
		instanceIndex = _lastInstanceCursor++;
		if (_lastInstanceCursor > _maxInstanceCount)
			return std::nullopt;
	}
	else
	{
		instanceIndex = _freeFrameIndices.back();
		_freeFrameIndices.pop_back();
	}

	CRenderInstancePool* renderInstancePool = nullptr;
	if (_motionType == ERenderInstanceMotionType::Static)
		renderInstancePool = _context->GetPool<CRenderInstanceStaticPool>();
	else if (_motionType == ERenderInstanceMotionType::Dynamic)
		renderInstancePool = _context->GetPool<CRenderInstanceDynamicPool>();

	const XRenderInstance::THandle& beginHandle = _frame.begin;
	const XRenderInstance::THandle curHandle =
		*renderInstancePool->GetHandle(beginHandle, instanceIndex);

	XRenderInstance& ri = *renderInstancePool->Get(curHandle);
	ri.SetIndexInInstancePack(instanceIndex);

	MarkDirty();

	return curHandle;
}

void triton::XRenderInstancePack::RemoveInstance(const XRenderInstance::THandle& instance)
{
	_instanceCount -= 1;

	CRenderInstancePool* renderInstancePool = nullptr;
	if (_motionType == ERenderInstanceMotionType::Static)
		renderInstancePool = _context->GetPool<CRenderInstanceStaticPool>();
	else if (_motionType == ERenderInstanceMotionType::Dynamic)
		renderInstancePool = _context->GetPool<CRenderInstanceDynamicPool>();

	XRenderInstance& ri = *renderInstancePool->Get(instance);
	_freeFrameIndices.push_back(ri.GetIndexInInstancePack());

	MarkDirty();
}

void triton::XRenderInstancePack::UpdateBuffer()
{
	if (_bDirtyBit == True)
		PackInstancesToStagingBuffer();
}

void triton::XRenderInstancePack::MarkDirty()
{
	_bDirtyBit = True;
}

void triton::XRenderInstancePack::PackInstancesToStagingBuffer()
{
	CRenderInstancePool* renderInstancePool = nullptr;
	if (_motionType == ERenderInstanceMotionType::Static)
		renderInstancePool = _context->GetPool<CRenderInstanceStaticPool>();
	else if (_motionType == ERenderInstanceMotionType::Dynamic)
		renderInstancePool = _context->GetPool<CRenderInstanceDynamicPool>();

	const usize beginIndex = renderInstancePool->GetPackedIndex(_frame.begin);
	const usize objectCount = _frame.count;
	usize aliveObjectCount = 0;
	for (usize objectIndex = 0; objectIndex < objectCount; objectIndex++)
	{
		boolean bIsObjectDeleted = False;
		for (usize i = 0; i < _freeFrameIndices.size(); i++)
		{
			if (objectIndex == _freeFrameIndices[i])
			{
				bIsObjectDeleted = True;
				break;
			}
		}

		if (bIsObjectDeleted == False)
		{
			const XRenderInstance::THandle handle = *renderInstancePool->GetHandle(_frame.begin, objectIndex);
			XRenderInstance& ri = *renderInstancePool->Get(handle);

			renderInstancePool->WriteToStaging(
				beginIndex + aliveObjectCount,
				ri
			);

			++aliveObjectCount;
		}
	}

	_bufferOffset = beginIndex;

	_bDirtyBit = False;
}