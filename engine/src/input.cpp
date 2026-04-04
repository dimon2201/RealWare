// input.cpp

#include "input.hpp"
#include "context.hpp"
#include "input_backend.hpp"
#include "graphics_context_backend.hpp"

using namespace types;

triton::cInputWindow::cInputWindow(cContext* context, const sInputBackendWindow& backendWindow)
    : iObject(context), _backendWindow(backendWindow) {}
    
void triton::cInputWindow::SwapBuffers()
{
    iInputBackend* input = _context->GetBackend<iInputBackend>();

    return input->SwapWindowBuffers(_backendWindow);
}

types::boolean triton::cInputWindow::IsWindowFocused() const
{
    iInputBackend* input = _context->GetBackend<iInputBackend>();

    return input->IsWindowFocused();
}

triton::cInputWindow::eRunState triton::cInputWindow::GetRunState()
{
    iInputBackend* input = _context->GetBackend<iInputBackend>();
    
    return input->GetWindowRunState(_backendWindow);
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
    _windows = new cStack<cInputWindow>(_context, cad);

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

        _windows->Push(*window);
    }
}

void triton::cInput::Shutdown()
{
    _context->Destroy<cStack<cInputWindow>>(_windows);
}

triton::cInputWindow* triton::cInput::CreatePlatformWindow(
    const std::string& title,
    const cVector2& size,
    types::boolean fullscreen
)
{
    iInputBackend* input = _context->GetBackend<iInputBackend>();
    
    iGraphicsContextBackend* gfxContextBackend = _context->GetBackend<iGraphicsContextBackend>();
    sInputBackendWindow ibw = input->CreatePlatformWindow(title, size, fullscreen);
    input->BindWindowContext(ibw);
    input->SetWindowSwapInterval(1);
    gfxContextBackend->CreateGraphicsContext();

    cInputWindow* window = _context->Create<cInputWindow>(_context, ibw);

    return window;
}

void triton::cInput::BindWindowContext(cInputWindow* window)
{
    iInputBackend* input = _context->GetBackend<iInputBackend>();
    input->BindWindowContext(window->GetBackendWindow());
}

void triton::cInput::DestroyWindow(cInputWindow* window)
{
    iInputBackend* input = _context->GetBackend<iInputBackend>();
    input->DestroyWindow(window->_backendWindow);

    _context->Destroy<cInputWindow>(window);
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