// render_instance_pack.cpp

#include "render_instance_pack.hpp"
#include "context.hpp"
#include "render_instance_static_pool.hpp"
#include "render_instance_dynamic_pool.hpp"
#include "render_instance_pack_pool.hpp"
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

	for (usize i = 0; i < maxInstanceCount; i++)
		_freeFrameIndices.push_back(i);
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

	CalculateInstanceAccessIndicesAndBufferOffset();

	return curHandle;
}

std::optional<std::vector<triton::XRenderInstance::THandle>> triton::XRenderInstancePack::AddInstances(usize count)
{
	if (_instanceCount + count > _maxInstanceCount)
		return std::nullopt;

	_instanceCount += count;

	CRenderInstancePool* renderInstancePool = nullptr;
	if (_motionType == ERenderInstanceMotionType::Static)
		renderInstancePool = _context->GetPool<CRenderInstanceStaticPool>();
	else if (_motionType == ERenderInstanceMotionType::Dynamic)
		renderInstancePool = _context->GetPool<CRenderInstanceDynamicPool>();

	std::vector<XRenderInstance::THandle> instances = {};

	for (usize i = 0; i < count; i++)
	{
		s32 instanceIndex = -1;
		if (_freeFrameIndices.empty())
		{
			return std::nullopt;
		}
		else
		{
			instanceIndex = _freeFrameIndices.back();
			_freeFrameIndices.pop_back();
		}

		const XRenderInstance::THandle& beginHandle = _frame.begin;
		const XRenderInstance::THandle curHandle =
			*renderInstancePool->GetHandle(beginHandle, instanceIndex);

		XRenderInstance& ri = *renderInstancePool->Get(curHandle);
		ri.SetIndexInInstancePack(instanceIndex);

		instances.push_back(curHandle);
	}

	CalculateInstanceAccessIndicesAndBufferOffset();

	return instances;
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

	CalculateInstanceAccessIndicesAndBufferOffset();
}

void triton::XRenderInstancePack::RemoveInstances(const std::vector<XRenderInstance::THandle>& instances)
{
	const usize count = instances.size();

	_instanceCount -= count;

	CRenderInstancePool* renderInstancePool = nullptr;
	if (_motionType == ERenderInstanceMotionType::Static)
		renderInstancePool = _context->GetPool<CRenderInstanceStaticPool>();
	else if (_motionType == ERenderInstanceMotionType::Dynamic)
		renderInstancePool = _context->GetPool<CRenderInstanceDynamicPool>();

	for (usize i = 0; i < count; i++)
	{
		XRenderInstance& ri = *renderInstancePool->Get(instances.at(i));
		_freeFrameIndices.push_back(ri.GetIndexInInstancePack());
	}

	CalculateInstanceAccessIndicesAndBufferOffset();
}

void triton::XRenderInstancePack::CalculateInstanceAccessIndicesAndBufferOffset()
{
	CRenderInstancePool* renderInstancePool = nullptr;
	if (_motionType == ERenderInstanceMotionType::Static)
		renderInstancePool = _context->GetPool<CRenderInstanceStaticPool>();
	else if (_motionType == ERenderInstanceMotionType::Dynamic)
		renderInstancePool = _context->GetPool<CRenderInstanceDynamicPool>();

	auto CheckAlive = [this](const s32 index) -> boolean {
		for (usize i = 0; i < _freeFrameIndices.size(); i++)
		{
			if (index == _freeFrameIndices.at(i))
				return False;
		}
		return True;
	};

	const usize objectCount = _frame.count;
	const usize beginIndex = renderInstancePool->GetPackedIndex(_frame.begin);
	for (usize i = 0; i < objectCount; i++)
	{
		if (CheckAlive(i) == False)
			continue;

		const XRenderInstance::THandle handle = *renderInstancePool->GetHandle(_frame.begin, i);
		XRenderInstance& ri = *renderInstancePool->Get(handle);

		s32 packedIndexInInstancePack = 0;
		const s32 indexInInstancePack = ri.GetIndexInInstancePack();
		for (usize index = 0; index < indexInInstancePack; index++)
		{
			if (CheckAlive(index) == True)
				++packedIndexInInstancePack;
		}

		ri.SetAccessIndex(beginIndex + packedIndexInInstancePack);
	}

	_bufferOffset = beginIndex;
}