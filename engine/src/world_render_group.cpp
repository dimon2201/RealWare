// world_render_group.cpp

#include "world_render_group.hpp"
#include "world_render_group_instance_pool.hpp"

using namespace types;

triton::XRenderGroupInstance::THandle triton::XRenderGroup::CreateInstance(usize maxInstanceCount)
{
	CRenderGroupInstancePool* renderGroupInstancePool = _context->GetPool<CRenderGroupInstancePool>();

	return *renderGroupInstancePool->Create(
		_group.renderDomain,
		_group.motionType,
		_group.sharedGeometry,
		_group.sharedMaterial,
		maxInstanceCount
	);
}

void triton::XRenderGroup::DestroyInstance(const XRenderGroupInstance::THandle& instance)
{
	_context->GetPool<CRenderGroupInstancePool>()->Destroy(instance);
}