// render_batch.hpp

#pragma once

#include "geometry_view.hpp"

namespace triton
{
	class CRenderBatch final
	{
		SGeometryView _geometry = {};

	public:
		explicit CRenderBatch(const SGeometryView& geometry);
	};
}