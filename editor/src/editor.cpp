// editor.cpp

#include "editor.hpp"
#include "context.hpp"
#include "widget_backend_win32.hpp"

void triton::editor::Init(cContext* context)
{
	context->RegisterBackend<IWidgetBackend>(new XWidgetBackendWin32(context));
}

void triton::editor::Free(cContext* context)
{
	// TODO: release backend here
}