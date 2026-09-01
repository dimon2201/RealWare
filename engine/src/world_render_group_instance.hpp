// world_render_group_instance.hpp

#pragma once

#include <vector>
#include "object.hpp"
#include "world_render_group_struct.hpp"
#include "render_instance.hpp"

namespace triton
{
	class XRenderGroupInstance : public iObject
	{
		SRenderGroupData _group;
		std::vector<XRenderInstance> _instances;

	public:
		explicit XRenderGroupInstance(cContext* context, types::s32 poolIndex) : iObject(context, poolIndex) {}

		~XRenderGroupInstance() override = default;

		inline ERenderInstanceMotionType GetMotionType() { return _group.motionType; }

		inline const SGeometryView& GetSharedGeometry() { return _group.sharedGeometry; }

		inline const XMaterial::THandle& GetSharedMaterial() { return _group.sharedMaterial; }

		inline const std::vector<XRenderInstance>& GetInstances() { return _instances; }

		struct THandle : public SHandle {};

		struct TGPULayout {};
	};
}