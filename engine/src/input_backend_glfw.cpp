// input_backend_glfw.hpp

#pragma once

#include <GLFW/glfw3.h>
#include "input_backend_glfw.hpp"
#include "input.hpp"
#include "context.hpp"
#include "graphics.hpp"
#include "render_subsystem.hpp"
#include "types.hpp"

using namespace types;

triton::cInputBackendGLFW::cInputBackendGLFW(cContext* context) : iInputBackend(context)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

triton::sInputBackendWindow triton::cInputBackendGLFW::CreatePlatformWindow(
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

void triton::cInputBackendGLFW::DestroyWindow(sInputBackendWindow& window)
{
    if (window.instance == 0)
        return;

    glfwDestroyWindow((GLFWwindow*)window.instance);
}

void triton::cInputBackendGLFW::ResizeWindow(sInputBackendWindow& window, const cVector2& newSize)
{
    if (window.instance == 0)
        return;

    window.size = cVector2(newSize.GetX(), newSize.GetY());
}

void triton::cInputBackendGLFW::PollEvents()
{
    glfwPollEvents();
}

void* triton::cInputBackendGLFW::GetWindowWin32Handle(sInputBackendWindow& window)
{
    if (window.instance == 0)
        return nullptr;

    return nullptr; //(void*)glfwGetWin32Window((GLFWwindow*)window.instance);
}

triton::cInputWindow::eRunState triton::cInputBackendGLFW::GetWindowRunState(sInputBackendWindow& window)
{
    cInputWindow::eRunState runState = cInputWindow::eRunState::OPENED;

    int flag = glfwWindowShouldClose((GLFWwindow*)window.instance);
    if (flag)
        runState = cInputWindow::eRunState::CLOSED;

    return runState;
}

types::boolean triton::cInputBackendGLFW::GetKeyPressed(qword keyCode)
{
    return _keys[keyCode];
}

types::boolean triton::cInputBackendGLFW::GetMouseKeyPressed(qword keyCode)
{
    return _mouseKeys[keyCode];
}

triton::cVector2 triton::cInputBackendGLFW::GetCursorPosition(sInputBackendWindow& window)
{
    double xpos = 0, ypos = 0;
    glfwGetCursorPos(
        (GLFWwindow*)window.instance,
        &xpos,
        &ypos
    );

    return cVector2(xpos, ypos);
}

void triton::cInputBackendGLFW::SetKeyPressed(qword keyCode, types::boolean isPressed)
{
    _keys[keyCode] = isPressed;
}

void triton::cInputBackendGLFW::SetMouseKeyPressed(qword keyCode, types::boolean isPressed)
{
    _mouseKeys[keyCode] = isPressed;
}

void triton::cInputBackendGLFW::SetWindowFocus(types::boolean isFocused)
{
    _isFocused = isFocused;
}

void triton::cInputBackendGLFW::SetWindowCursorPosition(const cVector2& cursorPosition)
{
    _cursorPosition = cursorPosition;
}

void triton::cInputBackendGLFW::SetWindowSwapInterval(types::usize interval)
{
    glfwSwapInterval(interval);
}

triton::cVector2 triton::cInputBackendGLFW::GetMonitorSize()
{
    return cVector2(0.0f); //cVector2(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
}

types::boolean triton::cInputBackendGLFW::IsWindowFocused()
{
    return _isFocused;
}

void triton::cInputBackendGLFW::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    const cpuword keyBufferMask = 0xFF;
    key &= keyBufferMask;

    cContext* context = (cContext*)glfwGetWindowUserPointer(window);
    iInputBackend* input = context->GetBackend<iInputBackend>();

    if (action == GLFW_PRESS)
        input->SetKeyPressed(key, K_TRUE);
    else if (action == GLFW_RELEASE)
        input->SetKeyPressed(key, K_FALSE);
}

void triton::cInputBackendGLFW::WindowFocusCallback(GLFWwindow* window, int focused)
{
    cContext* context = (cContext*)glfwGetWindowUserPointer(window);
    iInputBackend* input = context->GetBackend<iInputBackend>();

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

void triton::cInputBackendGLFW::WindowSizeCallback(GLFWwindow* window, int width, int height)
{
    cContext* context = (cContext*)glfwGetWindowUserPointer(window);
    XRenderSubsystem* renderSubsystem = context->GetSubsystem<XRenderSubsystem>();
    cInput* input = context->GetSubsystem<cInput>();

    input->ResizeWindows(cVector2(width, height));

    // TODO: explore WTF is RESIZE_RENDER_TARGETS
    /*SRenderCommand cmd = SRenderCommand(
        ERenderCommand::RESIZE_RENDER_TARGETS,
        width,
        height,
        0,
        0
    );
    renderSubsystem->PushCommand(cmd);*/
}

void triton::cInputBackendGLFW::CursorCallback(GLFWwindow* window, double xpos, double ypos)
{
    cContext* context = (cContext*)glfwGetWindowUserPointer(window);
    iInputBackend* input = context->GetBackend<iInputBackend>();

    input->SetWindowCursorPosition(cVector2(xpos, ypos));
}

void triton::cInputBackendGLFW::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    cContext* context = (cContext*)glfwGetWindowUserPointer(window);
    iInputBackend* input = context->GetBackend<iInputBackend>();

    if (action == GLFW_RELEASE)
        input->SetMouseKeyPressed(button, 0);
    else if (action == GLFW_PRESS)
        input->SetMouseKeyPressed(button, 1);
}