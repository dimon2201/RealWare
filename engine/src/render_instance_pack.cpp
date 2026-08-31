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
	const XMaterial::THandle& sharedMaterial,
	types::usize maxInstanceCount
) :
	iObject(context, poolIndex),
	_bufferOffset(0),
	_instanceCount(0),
	_motionType(motionType),
	_sharedGeometry(sharedGeometry),
	_sharedMaterial(sharedMaterial)
{
	CRenderInstancePool* renderInstancePool = nullptr;
	if (_motionType == ERenderInstanceMotionType::Static)
		renderInstancePool = _context->GetPool<CRenderInstanceStaticPool>();
	else if (_motionType == ERenderInstanceMotionType::Dynamic)
		renderInstancePool = _context->GetPool<CRenderInstanceDynamicPool>();

	_frame = *renderInstancePool->CreateFrame(maxInstanceCount);
	_bufferOffset = renderInstancePool->GetPackedIndex(_frame.begin);
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

	CRenderInstancePool* renderInstancePool = nullptr;
	if (_motionType == ERenderInstanceMotionType::Static)
		renderInstancePool = _context->GetPool<CRenderInstanceStaticPool>();
	else if (_motionType == ERenderInstanceMotionType::Dynamic)
		renderInstancePool = _context->GetPool<CRenderInstanceDynamicPool>();

	const usize instanceIndex = _instanceCount++;

	const XRenderInstance::THandle& beginHandle = _frame.begin;
	const XRenderInstance::THandle currentHandle =
		*renderInstancePool->GetHandle(beginHandle, instanceIndex);

	XRenderInstance& ri = *renderInstancePool->Get(currentHandle);
	ri.SetMotionType(_motionType);
	ri.SetIndexInInstancePack(instanceIndex);
	ri.SetAccessIndex(_bufferOffset + instanceIndex);

	return currentHandle;
}

void triton::XRenderInstancePack::RemoveInstance(const XRenderInstance::THandle& instance)
{
	CRenderInstancePool* renderInstancePool = nullptr;
	if (_motionType == ERenderInstanceMotionType::Static)
		renderInstancePool = _context->GetPool<CRenderInstanceStaticPool>();
	else if (_motionType == ERenderInstanceMotionType::Dynamic)
		renderInstancePool = _context->GetPool<CRenderInstanceDynamicPool>();
	
	if (_instanceCount > 1)
	{
		const usize lastInstanceIndex = _instanceCount - 1;

		const XRenderInstance::THandle instanceCurrentHandle = instance;
		const XRenderInstance::THandle instanceLastHandle =
			*renderInstancePool->GetHandle(_frame.begin, lastInstanceIndex);

		XRenderInstance& instanceCurrent = *renderInstancePool->Get(instanceCurrentHandle);
		XRenderInstance& instanceLast = *renderInstancePool->Get(instanceLastHandle);

		const usize currentInstanceIndex = instanceCurrent.GetIndexInInstancePack();

		instanceCurrent = instanceLast;
		instanceCurrent.SetIndexInInstancePack(currentInstanceIndex);
		instanceCurrent.SetAccessIndex(_bufferOffset + currentInstanceIndex);

		renderInstancePool->WriteToStaging(
			instanceCurrent.GetAccessIndex(),
			instanceCurrent
		);
	}

	if (_instanceCount)
		--_instanceCount;
}