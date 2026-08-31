// world_render_group.hpp

#pragma once

#include "render_instance_motion_type.hpp"
#include "geometry_view.hpp"
#include "material.hpp"
#include "object.hpp"

namespace triton
{
	class XRenderGroup : public iObject
	{
		ERenderInstanceMotionType _motionType = ERenderInstanceMotionType::Unknown;
		SGeometryView _sharedGeometry;
		XMaterial::THandle _sharedMaterial;

	public:
		explicit XRenderGroup(cContext* context, types::s32 poolIndex) : iObject(context, poolIndex) {}

		~XRenderGroup() override = default;

		inline ERenderInstanceMotionType GetMotionType() { return _motionType; }

		inline const SGeometryView& GetSharedGeometry() { return _sharedGeometry; }

		inline const XMaterial::THandle& GetSharedMaterial() { return _sharedMaterial; }

		struct THandle : public SHandle {};

		struct TGPULayout {};
	};
}