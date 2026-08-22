// vertex.hpp

#pragma once

#include "math.hpp"
#include "handle.hpp"
#include "types.hpp"

namespace triton
{
	struct SRigidVertexGPULayout final
	{
		cVector3 position = cVector3(0.0f);
		cVector2 texcoord = cVector2(0.0f);
		cVector3 normal = cVector3(0.0f);
		cVector4 tangent = cVector4(0.0f);
	};

	struct SSkinnedVertexGPULayout final
	{
		cVector3 position = cVector3(0.0f);
		cVector2 texcoord = cVector2(0.0f);
		cVector3 normal = cVector3(0.0f);
		cVector4 tangent = cVector4(0.0f);
		types::u32 boneIndices[4] = {};
		types::f32 boneWeights[4] = {};
	};
}