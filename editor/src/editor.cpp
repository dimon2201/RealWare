// editor.cpp

#include "editor.hpp"
#include "context.hpp"
#include "widget_backend_win32.hpp"
#include "math.hpp"
#include "canvas_widget.hpp"
#include "types.hpp"

using namespace triton;
using namespace types;

void triton::editor::XEditor::Initialize()
{
	_context->CreateMemoryAllocator(); // TODO: add check to CreateMemoryAllocator() to prevent double allocation
	_context->RegisterBackend<IWidgetBackend>(new XWidgetBackendWin32(_context));
}

void triton::editor::XEditor::Shutdown()
{
	// TODO: release backend here
}

void triton::editor::XEditor::Run()
{
	IWidgetBackend* wb = _context->GetBackend<IWidgetBackend>();
	wb->CreateCanvas(
		"MyWindow",
		"MyWindow",
		cVector2(640, 480)
	);

	while (K_TRUE)
	{
		boolean result = wb->ProcessMessage();
		if (result == K_FALSE)
			break;
	}
}