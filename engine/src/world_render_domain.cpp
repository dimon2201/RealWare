// world_render_domain.cpp

#include "world_render_domain.hpp"
#include "world_render_domain_pool.hpp"
#include "world_render_group_instance_pool.hpp"
#include "world_render_group_pool.hpp"

using namespace types;

std::optional<triton::SHandle> triton::XRenderDomain::CreateRenderGroup(
	ERenderInstanceMotionType motionType,
	const SGeometryView& geometry,
	const SHandle& material
)
{
	CRenderGroupPool* renderGroupPool = _context->GetPool<CRenderGroupPool>();

	SHandle renderGroup = *renderGroupPool->Create(
		(const SHandle&)_context->GetPool<CRenderDomainPool>()->GetHandle(_poolIndex),
		motionType,
		geometry,
		(const XMaterial::THandle&)material
	);

	return renderGroup;
}

void triton::XRenderDomain::RemoveRenderGroup(const SHandle& renderGroup)
{
	_context->GetPool<CRenderGroupPool>()->Destroy((XRenderGroup::THandle)renderGroup);
}

void triton::XRenderDomain::QueryRenderGroupInstances()
{
	_renderGroupInstances.clear();

	CRenderGroupInstancePool* renderGroupInstancePool = _context->GetPool<CRenderGroupInstancePool>();

	XRenderDomain::THandle thisHandle = _context->GetPool<CRenderDomainPool>()->GetHandle(this->_poolIndex);
	SBufferView<XRenderGroupInstance> renderGroupInstancesBufferView = renderGroupInstancePool->GetData();

	for (usize i = 0; i < renderGroupInstancesBufferView.elementCount; ++i)
	{
		XRenderGroupInstance& renderGroupInstance = renderGroupInstancesBufferView.elements[i];

		if (renderGroupInstance.GetRenderDomain() == thisHandle)
			_renderGroupInstances.push_back(renderGroupInstancePool->GetHandle(i));
	}
}