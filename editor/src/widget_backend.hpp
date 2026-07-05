// widget_backend.hpp

#pragma once

#include <string>
#include "backend.hpp"

namespace triton
{
	class cContext;
}

namespace triton::editor
{
	struct SWidgetCanvas;
	class cVector2;

	class IWidgetBackend : public iBackend
	{
		TRITON_OBJECT(XWidgetBackend)

	public:
		explicit IWidgetBackend(cContext* context) : iBackend(context) {}
		~IWidgetBackend() override = default;

		virtual SWidgetCanvas CreateCanvas(
			const std::string& uniqueName,
			const std::string& title,
			const cVector2& size
		) = 0;
	};
}