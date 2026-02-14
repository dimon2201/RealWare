// input.cpp

#include "input.hpp"
#include "context.hpp"
#include "input_backend.hpp"
#include "render_context.hpp"

using namespace types;

triton::cInputWindow::cInputWindow(cContext* context, const sInputBackendWindow& backendWindow)
    : iObject(context), _backendWindow(backendWindow) {}
    
types::boolean triton::cInputWindow::IsWindowFocused() const
{
    iInputBackend* input = _context->GetSubsystem<iInputBackend>();

    return input->IsWindowFocused();
}

const triton::cVector2& triton::cInputWindow::GetSize() const
{
    return _backendWindow.size;
}

triton::cInput::cInput(cContext* context) : iObject(context) {}

triton::cInputWindow* triton::cInput::CreatePlatformWindow(
    const std::string& title,
    const cVector2& size,
    types::boolean fullscreen
)
{
    iInputBackend* input = _context->GetBackend<iInputBackend>();
    iGraphicsBackend* gfx = _context->GetSubsystem<iGraphicsBackend>();
    sInputBackendWindow ibw = input->CreatePlatformWindow(title, size, fullscreen);
    gfx->BindContext((void*)ibw.instance);

    return _context->Create<cInputWindow>(_context, ibw);
}

void triton::cInput::DestroyWindow(cInputWindow* window)
{
    iInputBackend* input = _context->GetSubsystem<iInputBackend>();
    input->DestroyWindow(window->_backendWindow);

    _context->Destroy<cInputWindow>(window);
}

triton::cVector2 triton::cInput::GetMonitorSize() const
{
    iInputBackend* input = _context->GetSubsystem<iInputBackend>();

    return input->GetMonitorSize();
}

types::boolean triton::cInput::GetKeyPressed(types::qword keyCode) const
{
    iInputBackend* input = _context->GetSubsystem<iInputBackend>();

    return input->GetKeyPressed(keyCode);
}

types::boolean triton::cInput::GetMouseKeyPressed(types::qword keyCode) const
{
    iInputBackend* input = _context->GetSubsystem<iInputBackend>();

    return input->GetMouseKeyPressed(keyCode);
}