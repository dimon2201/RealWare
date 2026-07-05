// widget_backend.hpp

#pragma once

#include "backend.hpp"

namespace triton
{
	class cContext;
}

namespace triton::editor
{
	struct SWidgetCanvas;

	class IWidgetBackend : public iBackend
	{
		TRITON_OBJECT(XWidgetBackend)

	public:
		explicit IWidgetBackend(cContext* context) : iBackend(context) {}
		~IWidgetBackend() override = default;

		virtual SWidgetCanvas CreateCanvas() = 0;
	};
}