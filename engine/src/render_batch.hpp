// render_batch.hpp

#pragma once

namespace triton
{
	class SGeometryView;

	class CRenderBatch final
	{
		SGeometryView _geometry = {};

	public:
		explicit CRenderBatch(const SGeometryView& geometry);
	};
}