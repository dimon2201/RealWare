// render_data.hpp

#pragma once

#include "render_instance.hpp"
#include "components.hpp"

namespace triton
{
	class SRenderData final
	{
	public:
		cStack<ecs::components::SRenderInstanceComponent>* _renderInstances = nullptr;
	};
}