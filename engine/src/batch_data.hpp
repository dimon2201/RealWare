// batch_data.hpp

#pragma once

#include "geometry_view.hpp"
#include "render_instance_data.hpp"
#include "handle.hpp"
#include "types.hpp"

namespace triton
{
	struct SBatchData final
	{
		struct THandle : public SHandle {};

		struct TGPULayout {};

		ERenderInstanceMotionType motionType = ERenderInstanceMotionType::Static;
		SGeometryView sharedGeometry = {};
		types::usize bufferOffset = 0;
		types::usize instanceCount = 0;
	};
}