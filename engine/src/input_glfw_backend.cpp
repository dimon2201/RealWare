// input_backend.hpp

#pragma once

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include "input_glfw_backend.hpp"
#include "context.hpp"
#include "graphics.hpp"
#include "types.hpp"

using namespace types;

triton::cInputGLFWBackend::cInputGLFWBackend(cContext* context) : iInputBackend(context)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

triton::sInputBackendWindow triton::cInputGLFWBackend::CreatePlatformWindow(
    const std::string& title,
    const cVector2& size,
    types::boolean fullscreen
)
{
    sInputBackendWindow ibw = {};
    ibw.title = title;
    ibw.size = size;
    ibw.fullscreen = fullscreen;

    if (fullscreen == K_FALSE)
    {
        ibw.instance = (types::qword)glfwCreateWindow(ibw.size.GetX(), ibw.size.GetY(), ibw.title.c_str(), nullptr, nullptr);
    }
    else
    {
        glfwWindowHint(GLFW_DECORATED, 0);

        ibw.size = GetMonitorSize();
        ibw.instance = (types::qword)glfwCreateWindow(ibw.size.GetX(), ibw.size.GetY(), ibw.title.c_str(), glfwGetPrimaryMonitor(), nullptr);
    }

    if (!ibw.instance)
    {
        Print("Error: incompatible GL version!");
        return ibw;
    }

    glfwSetWindowUserPointer((GLFWwindow*)ibw.instance, _context);

    glfwSetKeyCallback((GLFWwindow*)ibw.instance, &KeyCallback);
    glfwSetWindowFocusCallback((GLFWwindow*)ibw.instance, &WindowFocusCallback);
    glfwSetWindowSizeCallback((GLFWwindow*)ibw.instance, &WindowSizeCallback);
    glfwSetCursorPosCallback((GLFWwindow*)ibw.instance, &CursorCallback);
    glfwSetMouseButtonCallback((GLFWwindow*)ibw.instance, &MouseButtonCallback);

    return ibw;
}

void triton::cInputGLFWBackend::DestroyWindow(sInputBackendWindow& window)
{
    if (window.instance == 0)
        return;

    glfwDestroyWindow((GLFWwindow*)window.instance);
}

void triton::cInputGLFWBackend::ResizeWindow(sInputBackendWindow& window, const cVector2& newSize)
{
    if (window.instance == 0)
        return;

    window.size = cVector2(newSize.GetX(), newSize.GetY());
}

void triton::cInputGLFWBackend::SwapWindowBuffers(sInputBackendWindow& window)
{
    if (window.instance == 0)
        return;

    glfwSwapBuffers((GLFWwindow*)window.instance);
}
    
void triton::cInputGLFWBackend::PollEvents()
{
    glfwPollEvents();
}

void* triton::cInputGLFWBackend::GetWindowWin32Handle(sInputBackendWindow& window)
{
    if (window.instance == 0)
        return nullptr;

    return (void*)glfwGetWin32Window((GLFWwindow*)window.instance);
}

types::boolean triton::cInputGLFWBackend::GetKeyPressed(qword keyCode)
{
    return _keys[keyCode];
}

types::boolean triton::cInputGLFWBackend::GetMouseKeyPressed(qword keyCode)
{
    return _mouseKeys[keyCode];
}

void triton::cInputGLFWBackend::SetKeyPressed(qword keyCode, types::boolean isPressed)
{
    _keys[keyCode] = isPressed;
}

void triton::cInputGLFWBackend::SetMouseKeyPressed(qword keyCode, types::boolean isPressed)
{
    _mouseKeys[keyCode] = isPressed;
}

void triton::cInputGLFWBackend::SetWindowFocus(types::boolean isFocused)
{
    _isFocused = isFocused;
}

void triton::cInputGLFWBackend::SetWindowCursorPosition(const cVector2& cursorPosition)
{
    _cursorPosition = cursorPosition;
}

triton::cVector2 triton::cInputGLFWBackend::GetMonitorSize()
{
    return cVector2(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
}

types::boolean triton::cInputGLFWBackend::IsWindowFocused()
{
    return _isFocused;
}

void triton::cInputGLFWBackend::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    const cpuword keyBufferMask = 0xFF;
    key &= keyBufferMask;

    cContext* context = (cContext*)glfwGetWindowUserPointer(window);
    iInputBackend* input = context->GetSubsystem<iInputBackend>();

    if (action == GLFW_PRESS)
        input->SetKeyPressed(key, K_TRUE);
    else if (action == GLFW_RELEASE)
        input->SetKeyPressed(key, K_FALSE);
}

void triton::cInputGLFWBackend::WindowFocusCallback(GLFWwindow* window, int focused)
{
    cContext* context = (cContext*)glfwGetWindowUserPointer(window);
    iInputBackend* input = context->GetSubsystem<iInputBackend>();

    if (focused)
    {
        if (input->IsWindowFocused() == K_FALSE)
            input->SetWindowFocus(K_TRUE);
    }
    else
    {
        input->SetWindowFocus(K_FALSE);
    }
}

void triton::cInputGLFWBackend::WindowSizeCallback(GLFWwindow* window, int width, int height)
{
    cContext* context = (cContext*)glfwGetWindowUserPointer(window);
    iInputBackend* input = context->GetSubsystem<iInputBackend>();
    cGraphics* gfx = context->GetSubsystem<cGraphics>();

    // TODO: resize specific cInputWindow based on GLFWwindow
    // input->ResizeWindow(ibw, cVector2(width, height));

    gfx->ResizeRenderTargets(glm::vec2(width, height));
}

void triton::cInputGLFWBackend::CursorCallback(GLFWwindow* window, double xpos, double ypos)
{
    cContext* context = (cContext*)glfwGetWindowUserPointer(window);
    iInputBackend* input = context->GetSubsystem<iInputBackend>();

    input->SetWindowCursorPosition(cVector2(xpos, ypos));
}

void triton::cInputGLFWBackend::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    cContext* context = (cContext*)glfwGetWindowUserPointer(window);
    iInputBackend* input = context->GetSubsystem<iInputBackend>();

    if (action == GLFW_RELEASE)
        input->SetMouseKeyPressed(button, 0);
    else if (action == GLFW_PRESS)
        input->SetMouseKeyPressed(button, 1);
}