// world_render_group_instance.cpp

#include "world_render_group_instance.hpp"
#include "render_instance_pool.hpp"
#include "render_instance_static_pool.hpp"
#include "render_instance_dynamic_pool.hpp"

using namespace types;

triton::XRenderGroupInstance::XRenderGroupInstance(
	cContext* context,
	s32 poolIndex,
	const SHandle& renderDomain,
	ERenderInstanceMotionType motionType,
	const SGeometryView& sharedGeometry,
	const XMaterial::THandle& sharedMaterial,
	usize maxInstanceCount
) :
	iObject(context, poolIndex),
	_group(renderDomain, motionType, sharedGeometry, sharedMaterial),
	_maxInstanceCount(maxInstanceCount)
{
	CRenderInstancePool* renderInstancePool = nullptr;

	if (_group.motionType == ERenderInstanceMotionType::Static)
		renderInstancePool = _context->GetPool<CRenderInstanceStaticPool>();
	else if (_group.motionType == ERenderInstanceMotionType::Dynamic)
		renderInstancePool = _context->GetPool<CRenderInstanceDynamicPool>();

	_maxInstanceCount = maxInstanceCount;
	_instanceCount = 0;

	_instances = *renderInstancePool->CreateFrame(_maxInstanceCount);
}

triton::XRenderGroupInstance::~XRenderGroupInstance()
{
	CRenderInstancePool* renderInstancePool = nullptr;

	if (_group.motionType == ERenderInstanceMotionType::Static)
		renderInstancePool = _context->GetPool<CRenderInstanceStaticPool>();
	else if (_group.motionType == ERenderInstanceMotionType::Dynamic)
		renderInstancePool = _context->GetPool<CRenderInstanceDynamicPool>();

	renderInstancePool->DestroyFrame(_instances);

	_instanceCount = 0;
}

const triton::XRenderInstance::THandle& triton::XRenderGroupInstance::AddInstance()
{
	if (_instanceCount >= _maxInstanceCount)
		return XRenderInstance::THandle();

	CRenderInstancePool* renderInstancePool = nullptr;
	
	if (_group.motionType == ERenderInstanceMotionType::Static)
		renderInstancePool = _context->GetPool<CRenderInstanceStaticPool>();
	else if (_group.motionType == ERenderInstanceMotionType::Dynamic)
		renderInstancePool = _context->GetPool<CRenderInstanceDynamicPool>();

	const usize indexInGroup = _instanceCount;

	XRenderInstance& instance = *renderInstancePool->GetFrame(_instances, indexInGroup);
	instance = XRenderInstance(_context, instance.GetPoolIndex(), indexInGroup);

	++_instanceCount;

	XRenderInstance::THandle handle = renderInstancePool->GetHandle(_instances.begin.index + indexInGroup);

	return handle;
}

void triton::XRenderGroupInstance::RemoveInstance(const XRenderInstance::THandle& instanceHandle)
{
	if (!_instanceCount)
		return;

	CRenderInstancePool* renderInstancePool = nullptr;

	if (_group.motionType == ERenderInstanceMotionType::Static)
		renderInstancePool = _context->GetPool<CRenderInstanceStaticPool>();
	else if (_group.motionType == ERenderInstanceMotionType::Dynamic)
		renderInstancePool = _context->GetPool<CRenderInstanceDynamicPool>();

	if (_instanceCount > 1)
	{
		XRenderInstance& curInstance = *renderInstancePool->Get(instanceHandle);

		const usize curIndexInGroup = curInstance.GetIndexInGroup();
		const usize lastIndexInGroup = _instanceCount - 1;

		XRenderInstance& lastInstance = *renderInstancePool->GetFrame(_instances, lastIndexInGroup);

		curInstance = lastInstance;
	}

	--_instanceCount;
}