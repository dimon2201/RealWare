// canvas.hpp

#pragma once

#include "object.hpp"

namespace triton
{
	class cContext;
}

namespace triton::editor
{
	struct SWidgetCanvas
	{
	};

	class XCanvas : public iObject
	{
		TRITON_OBJECT(XCanvas)

		SWidgetCanvas _canvas = {};

	public:
		explicit XCanvas(cContext* context);
		~XCanvas() override;
	};
}