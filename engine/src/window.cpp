// window.cpp

#include "window.hpp"
#include "context.hpp"
#include "input_backend.hpp"

using namespace types;

triton::CWindow::CWindow(
	cContext* context,
	const std::string& title,
	const cVector2& size,
	boolean fullscreen,
	EAvailableGraphicsBackend graphicsBackend
) : _context(context)
{
	_backendWindow = _context->GetBackend<IInputBackend>()->CreateBackendWindow(
		title,
		size,
		fullscreen,
		graphicsBackend
	);
}

triton::CWindow::~CWindow()
{
	_context->GetBackend<IInputBackend>()->DestroyBackendWindow(_backendWindow);
}