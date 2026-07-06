// widget_backend.hpp

#pragma once

#include <string>
#include "backend.hpp"
#include "types.hpp"

namespace triton
{
	class cContext;
	class cVector2;
}

namespace triton::editor
{
	struct SWidgetCanvas;

	class IWidgetBackend : public iBackend
	{
		TRITON_OBJECT(IWidgetBackend)

	public:
		explicit IWidgetBackend(cContext* context) : iBackend(context) {}
		~IWidgetBackend() override = default;

		virtual SWidgetCanvas CreateCanvas(
			const std::string& uniqueName,
			const std::string& title,
			const triton::cVector2& size
		) = 0;
		virtual types::boolean ProcessMessage() = 0;
	};
}