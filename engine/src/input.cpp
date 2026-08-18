// input.cpp

#include "input.hpp"
#include "context.hpp"
#include "input_backend.hpp"
#include "thread_guard.hpp"
#include "dynamic_array.hpp"

using namespace types;

triton::cInputWindow::cInputWindow(cContext* context, const sInputBackendWindow& backendWindow)
    : iObject(context), _backendWindow(backendWindow) {}
    
types::boolean triton::cInputWindow::IsWindowFocused() const
{
    iInputBackend* input = _context->GetBackend<iInputBackend>();

    return input->IsWindowFocused();
}

const triton::cVector2& triton::cInputWindow::GetSize() const
{
    return _backendWindow.size;
}

triton::cInput::cInput(cContext* context) : iObject(context) {}

void triton::cInput::Initialize()
{
    if (_windows != nullptr)
        return;

    const sCapabilities* caps = _context->GetSubsystem<cEngine>()->GetCapabilities();
    sChunkAllocatorDescriptor cad = {};
    cad.chunkByteSize = caps->hashTableChunkByteSize;
    cad.maxChunkCount = caps->hashTableMaxChunkCount;
    cad.hashTableSize = caps->hashTableSize;
    _windows = new std::vector<cInputWindow>();

    cInput* input = _context->GetSubsystem<cInput>();
    if (input == nullptr)
        return;

    for (usize i = 0; i < caps->windowCount; i++)
    {
        cInputWindow* window = input->CreatePlatformWindow(
            caps->windows[i].windowTitle,
            cVector2(caps->windows[i].windowWidth, caps->windows[i].windowHeight),
            caps->windows[i].fullscreen
        );

        _windows->push_back(*window);
    }
}

void triton::cInput::Shutdown()
{
    //_context->Destroy<XDynamicArray<cInputWindow>>(_windows);
}

triton::cInputWindow* triton::cInput::CreatePlatformWindow(
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

void triton::cInput::DestroyWindow(cInputWindow* window)
{
    iInputBackend* input = _context->GetBackend<iInputBackend>();
    input->DestroyWindow(window->_backendWindow);

    _context->Destroy<cInputWindow>(window);
}

void triton::cInput::ResizeWindows(const cVector2& newSize)
{
    iInputBackend* input = _context->GetBackend<iInputBackend>();

    for (usize i = 0; i < _windows->size(); i++)
        input->ResizeWindow(_windows->at(i).GetBackendWindow(), newSize);
}

triton::cVector2 triton::cInput::GetCursorPosition(cInputWindow* window)
{
    iInputBackend* input = _context->GetBackend<iInputBackend>();
    
    return input->GetCursorPosition(window->GetBackendWindow());
}

triton::cVector2 triton::cInput::GetMonitorSize() const
{
    iInputBackend* input = _context->GetBackend<iInputBackend>();

    return input->GetMonitorSize();
}

types::boolean triton::cInput::GetKeyPressed(types::qword keyCode) const
{
    iInputBackend* input = _context->GetBackend<iInputBackend>();

    return input->GetKeyPressed(keyCode);
}

types::boolean triton::cInput::GetMouseKeyPressed(types::qword keyCode) const
{
    iInputBackend* input = _context->GetBackend<iInputBackend>();

    return input->GetMouseKeyPressed(keyCode);
}