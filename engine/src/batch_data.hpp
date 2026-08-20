// batch_data.hpp

#pragma once

#include <vector>
#include "geometry_view.hpp"
#include "render_instance_motion_type.hpp"
#include "handle.hpp"
#include "vertex_buffer_format.hpp"
#include "render_instance_data.hpp"
#include "object_pool.hpp"
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
		types::usize lastCreatedInstanceCursor = 0;
		types::usize instanceCount = 0;
		types::usize maxReservedInstanceCount = 0;
		SObjectFrame<SStaticRenderInstanceData::THandle> staticsFrame;
		SObjectFrame<SDynamicRenderInstanceData::THandle> dynamicsFrame;
		std::vector<types::s32> freeFrameIndices;
	};
}