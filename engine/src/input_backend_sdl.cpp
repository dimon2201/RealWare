// input_backend_sdl.hpp

#pragma once

#include <SDL3/SDL.h>
#include "input_backend_sdl.hpp"
#include "input.hpp"
#include "context.hpp"
#include "graphics.hpp"
#include "render_subsystem.hpp"
#include "types.hpp"

using namespace types;

triton::cInputBackendSDL::cInputBackendSDL(cContext* context) : iInputBackend(context)
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        Print("Error: SDL_Init failed");
        return;
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#ifdef _DEBUG
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
}

triton::sInputBackendWindow triton::cInputBackendSDL::CreatePlatformWindow(
    const std::string& title,
    const cVector2& size,
    boolean fullscreen
)
{
    sInputBackendWindow ibw = {};
    ibw.title = title;
    ibw.size = size;
    ibw.fullscreen = fullscreen;

    if (fullscreen == K_FALSE)
    {
        ibw.instance = (qword)SDL_CreateWindow(
            ibw.title.c_str(),
            ibw.size.GetX(),
            ibw.size.GetY(),
            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
        );
    }
    else
    {
        // TODO: add fullscreen mode
    }

    if (!ibw.instance)
    {
        Print("Error: SDL_CreateWindow failed");
        return ibw;
    }

    return ibw;
}

void triton::cInputBackendSDL::DestroyWindow(sInputBackendWindow& window)
{
    if (window.instance == 0)
        return;

    SDL_DestroyWindow((SDL_Window*)window.instance);
}

void triton::cInputBackendSDL::ResizeWindow(sInputBackendWindow& window, const cVector2& newSize)
{
    if (window.instance == 0)
        return;

    window.size = cVector2(newSize.GetX(), newSize.GetY());
}

triton::SEvent triton::cInputBackendSDL::PollEvent()
{
    SEvent e = {};
    SDL_Event event = {};
    if (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_EVENT_KEY_DOWN:
            {
                e.type = EWindowEvent::KeyDown;
                e.argA = event.key.scancode;
                return e;
            }
            case SDL_EVENT_KEY_UP:
            {
                e.type = EWindowEvent::KeyUp;
                e.argA = event.key.scancode;
                return e;
            }
            case SDL_EVENT_WINDOW_FOCUS_GAINED:
            {
                e.type = EWindowEvent::FocusGained;
                return e;
            }
            case SDL_EVENT_WINDOW_FOCUS_LOST:
            {
                e.type = EWindowEvent::FocusLost;
                return e;
            }
            case SDL_EVENT_WINDOW_RESIZED:
            {
                e.type = EWindowEvent::Resized;
                e.argA = event.window.data1;
                e.argB = event.window.data2;
                return e;
            }
            case SDL_EVENT_MOUSE_MOTION:
            {
                e.type = EWindowEvent::MouseMotion;
                e.argA = event.motion.x;
                e.argB = event.motion.y;
                return e;
            }
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            {
                e.type = EWindowEvent::MouseButtonDown;
                e.argA = event.button.button;
                return e;
            }
            case SDL_EVENT_MOUSE_BUTTON_UP:
            {
                e.type = EWindowEvent::MouseButtonUp;
                e.argA = event.button.button;
                return e;
            }
            case SDL_EVENT_QUIT:
            {
                e.type = EWindowEvent::Quit;
                return e;
            }
        }
    }

    return e;
}

void triton::cInputBackendSDL::ProcessEvent(const SEvent& event)
{
    iInputBackend* input = _context->GetBackend<iInputBackend>();
    sInputBackendWindow& ibw = _context->GetSubsystem<cInput>()->GetWindows()->At(0).data->GetBackendWindow();
    switch (event.type)
    {
        case EWindowEvent::KeyDown:
        {
            input->SetKeyPressed(event.argA, K_TRUE);
            break;
        }

        case EWindowEvent::KeyUp:
        {
            input->SetKeyPressed(event.argA, K_FALSE);
            break;
        }

        case EWindowEvent::FocusGained:
        {
            if (input->IsWindowFocused() == K_FALSE)
                input->SetWindowFocus(K_TRUE);
            break;
        }

        case EWindowEvent::FocusLost:
        {
            input->SetWindowFocus(K_FALSE);
            break;
        }

        case EWindowEvent::Resized:
        {
            input->ResizeWindow(ibw, cVector2(event.argA, event.argB));
            break;
        }

        case EWindowEvent::MouseMotion:
        {
            input->SetWindowCursorPosition(cVector2(event.argA, event.argB));
            break;
        }

        case EWindowEvent::MouseButtonDown:
        {
            input->SetMouseKeyPressed(event.argA, K_TRUE);
            break;
        }

        case EWindowEvent::MouseButtonUp:
        {
            input->SetMouseKeyPressed(event.argB, K_FALSE);
            break;
        }
    }
}

void* triton::cInputBackendSDL::GetWindowWin32Handle(sInputBackendWindow& window)
{
    if (window.instance == 0)
        return nullptr;

    return nullptr; //(void*)glfwGetWin32Window((GLFWwindow*)window.instance);
}

boolean triton::cInputBackendSDL::GetKeyPressed(qword keyCode)
{
    return _keys[keyCode];
}

boolean triton::cInputBackendSDL::GetMouseKeyPressed(qword keyCode)
{
    return _mouseKeys[keyCode];
}

triton::cVector2 triton::cInputBackendSDL::GetCursorPosition(sInputBackendWindow& window)
{
    // TODO: Implement cursor position getter

    return cVector2(0.0f);
}

void triton::cInputBackendSDL::SetKeyPressed(qword keyCode, boolean isPressed)
{
    _keys[keyCode] = isPressed;
}

void triton::cInputBackendSDL::SetMouseKeyPressed(qword keyCode, boolean isPressed)
{
    _mouseKeys[keyCode] = isPressed;
}

void triton::cInputBackendSDL::SetWindowFocus(boolean isFocused)
{
    _isFocused = isFocused;
}

void triton::cInputBackendSDL::SetWindowCursorPosition(const cVector2& cursorPosition)
{
    _cursorPosition = cursorPosition;
}

void triton::cInputBackendSDL::SetVSync(cpuword flag)
{
    // TODO: Implement SDL VSync
}

triton::cVector2 triton::cInputBackendSDL::GetMonitorSize()
{
    return cVector2(0.0f); //cVector2(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
}

boolean triton::cInputBackendSDL::IsWindowFocused()
{
    return _isFocused;
}