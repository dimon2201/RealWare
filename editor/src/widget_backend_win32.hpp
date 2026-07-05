// widget_backend_win32.hpp

#pragma once

#include <string>
#include "widget_backend.hpp"

namespace triton
{
	class cContext;
}

namespace triton::editor
{
	class XWidgetBackendWin32 : public IWidgetBackend
	{
		TRITON_OBJECT(XWidgetBackendWin32)

	public:
		explicit XWidgetBackendWin32(cContext* context) : IWidgetBackend(context) {}
		~XWidgetBackendWin32() override = default;

		SWidgetCanvas CreateCanvas(
			const std::string& uniqueName,
			const std::string& title,
			const cVector2& size
		) override final;
	};
}