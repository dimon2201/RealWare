// world_render_group_instance.hpp

#pragma once

#include "object.hpp"
#include "world_render_group.hpp"

namespace triton
{
	class XRenderGroupInstance : public iObject
	{
		XRenderGroup::THandle _renderGroup;

	public:
		explicit XRenderGroupInstance(cContext* context, types::s32 poolIndex) : iObject(context, poolIndex) {}

		~XRenderGroupInstance() override = default;

		inline const XRenderGroup::THandle& GetRenderGroup() { return _renderGroup; }
	};
}