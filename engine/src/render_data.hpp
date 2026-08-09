// render_data.hpp

#pragma once

#include "render_instance_data.hpp"
#include "components.hpp"

namespace triton
{
	class SRenderData final
	{
	public:
		XDynamicArray<ecs::components::SRenderInstanceComponent>* _renderInstances = nullptr;
	};
}