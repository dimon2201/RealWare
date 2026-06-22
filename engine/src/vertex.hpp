// vertex.hpp

#pragma once

#include "math.hpp"

namespace triton
{
	struct SVertex
	{
		cVector3 position = cVector3(0.0f);
		cVector2 texcoord = cVector2(0.0f);
		cVector3 normal = cVector3(0.0f);
	};
}