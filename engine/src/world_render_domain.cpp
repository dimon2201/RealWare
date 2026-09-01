// world_render_domain.cpp

#include "world_render_domain.hpp"
#include "world_render_domain_pool.hpp"
#include "world_render_group_instance_pool.hpp"

using namespace types;

std::vector<triton::XRenderGroupInstance::THandle> triton::XRenderDomain::QueryRenderGroupInstances()
{
	CRenderGroupInstancePool* renderGroupInstancePool = _context->GetPool<CRenderGroupInstancePool>();

	std::vector<XRenderGroupInstance::THandle> queriedRenderGroupInstances;
	XRenderDomain::THandle thisHandle = _context->GetPool<CRenderDomainPool>()->GetHandle(this->_poolIndex);
	SBufferView<XRenderGroupInstance> renderGroupInstancesBufferView = renderGroupInstancePool->GetData();

	for (usize i = 0; i < renderGroupInstancesBufferView.elementCount; ++i)
	{
		XRenderGroupInstance& renderGroupInstance = renderGroupInstancesBufferView.elements[i];

		if (renderGroupInstance.GetRenderDomain() == thisHandle)
			queriedRenderGroupInstances.push_back(renderGroupInstancePool->GetHandle(i));
	}

	return queriedRenderGroupInstances;
}