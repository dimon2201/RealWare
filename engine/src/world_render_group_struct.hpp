// world_render_group_struct.hpp

#pragma once

#include "render_instance_motion_type.hpp"
#include "geometry_view.hpp"
#include "material.hpp"

namespace triton
{
	struct SRenderGroupData
	{
		SRenderGroupData() = default;

		SRenderGroupData(
			const SHandle& renderDomain,
			ERenderInstanceMotionType motionType,
			const SGeometryView& sharedGeometry,
			const XMaterial::THandle& sharedMaterial
		) :
			renderDomain(renderDomain),
			motionType(motionType),
			sharedGeometry(sharedGeometry),
			sharedMaterial(sharedMaterial) {}

		SHandle renderDomain;
		ERenderInstanceMotionType motionType = ERenderInstanceMotionType::Unknown;
		SGeometryView sharedGeometry;
		XMaterial::THandle sharedMaterial;
	};
}