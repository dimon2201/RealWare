// world_render_group.hpp

#pragma once

#include <vector>
#include "object.hpp"
#include "world_render_group_struct.hpp"
#include "world_render_group_instance.hpp"

namespace triton
{
	class XRenderGroup : public iObject
	{
		SRenderGroupData _group;
		std::vector<XRenderGroupInstance> _instances;

	public:
		explicit XRenderGroup(cContext* context, types::s32 poolIndex) : iObject(context, poolIndex) {}

		~XRenderGroup() override = default;

		inline ERenderInstanceMotionType GetMotionType() { return _group.motionType; }

		inline const SGeometryView& GetSharedGeometry() { return _group.sharedGeometry; }

		inline const XMaterial::THandle& GetSharedMaterial() { return _group.sharedMaterial; }

		inline const std::vector<XRenderGroupInstance>& GetInstances() { return _instances; }

		struct THandle : public SHandle {};

		struct TGPULayout {};
	};
}