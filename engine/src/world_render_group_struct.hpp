// world_render_group_struct.hpp

#pragma once

#include "render_instance_motion_type.hpp"
#include "geometry_view.hpp"
#include "material.hpp"
#include "camera.hpp"

namespace triton
{
	struct SRenderGroupData
	{
		XCamera::THandle camera;
		ERenderInstanceMotionType motionType = ERenderInstanceMotionType::Unknown;
		SGeometryView sharedGeometry;
		XMaterial::THandle sharedMaterial;
	};
}