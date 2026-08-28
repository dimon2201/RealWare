// render_pass_push_constants_struct.hpp

#pragma once

#include "math.hpp"
#include "types.hpp"

namespace triton
{
	struct SRenderPassGPUPushConstantsLayout
	{
		cVector3 cameraWorldPosition = cVector3(0.0f);
		types::dword instanceMotionType = 0;
		cMatrix4 cameraViewProjectionMatrix = cMatrix4(1.0f);
		types::f32 time = 0.0f;
	};
}