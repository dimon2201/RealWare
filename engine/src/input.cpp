// input.cpp

#include "input.hpp"
#include "context.hpp"
#include "input_backend.hpp"
#include "thread_guard.hpp"

using namespace types;

triton::cInputWindow::cInputWindow(cContext* context, const sInputBackendWindow& backendWindow)
    : _context(context), _backendWindow(backendWindow) {}
    
types::boolean triton::cInputWindow::IsWindowFocused() const
{
    iInputBackend* input = _context->GetBackend<iInputBackend>();

    return input->IsWindowFocused();
}

const triton::cVector2& triton::cInputWindow::GetSize() const
{
    return _backendWindow.size;
}

triton::CInput::CInput(cContext* context) : CSubsystem(context) {}

void triton::CInput::Initialize(const sCapabilities& caps)
{
    if (_windows != nullptr)
        return;

    _windows = new std::vector<cInputWindow>();

    CInput* input = _context->GetSubsystem<CInput>();
    if (input == nullptr)
        return;

    for (usize i = 0; i < caps.windowCount; i++)
    {
        cInputWindow* window = input->CreatePlatformWindow(
            caps.windows[i].windowTitle,
            cVector2(caps.windows[i].windowWidth, caps.windows[i].windowHeight),
            caps.windows[i].fullscreen
        );

        _windows->push_back(*window);
    }
}

void triton::CInput::Shutdown()
{
    //_context->Destroy<XDynamicArray<cInputWindow>>(_windows);
}

triton::cInputWindow* triton::CInput::CreatePlatformWindow(
    const std::string& title,
    const cVector2& size,
    types::boolean fullscreen
)
{
    iInputBackend* input = _context->GetBackend<iInputBackend>();
    sInputBackendWindow ibw = input->CreatePlatformWindow(title, size, fullscreen);
    // TODO: Set SDL VSync here

    cInputWindow* window = _context->Create<cInputWindow>(_context, ibw);

    return window;
}

void triton::CInput::DestroyWindow(cInputWindow* window)
{
    iInputBackend* input = _context->GetBackend<iInputBackend>();
    input->DestroyWindow(window->GetBackendWindow());

    _context->Destroy<cInputWindow>(window);
}

void triton::CInput::ResizeWindows(const cVector2& newSize)
{
    iInputBackend* input = _context->GetBackend<iInputBackend>();

    for (usize i = 0; i < _windows->size(); i++)
        input->ResizeWindow(_windows->at(i).GetBackendWindow(), newSize);
}

triton::cVector2 triton::CInput::GetCursorPosition(cInputWindow* window)
{
    iInputBackend* input = _context->GetBackend<iInputBackend>();
    
    return input->GetCursorPosition(window->GetBackendWindow());
}

triton::cVector2 triton::CInput::GetMonitorSize() const
{
    iInputBackend* input = _context->GetBackend<iInputBackend>();

    return input->GetMonitorSize();
}

types::boolean triton::CInput::GetKeyPressed(types::qword keyCode) const
{
    iInputBackend* input = _context->GetBackend<iInputBackend>();

    return input->GetKeyPressed(keyCode);
}

types::boolean triton::CInput::GetMouseKeyPressed(types::qword keyCode) const
{
    iInputBackend* input = _context->GetBackend<iInputBackend>();

    return input->GetMouseKeyPressed(keyCode);
}