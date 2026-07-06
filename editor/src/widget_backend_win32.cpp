// widget_backend_win32.cpp

#include <windows.h>
#include "widget_backend_win32.hpp"
#include "canvas_widget.hpp"
#include "math.hpp"

using namespace triton;
using namespace types;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg,
    WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

triton::editor::SWidgetCanvas triton::editor::XWidgetBackendWin32::CreateCanvas(
    const std::string& uniqueName,
	const std::string& title,
	const triton::cVector2& size
)
{
    HINSTANCE hInstance = GetModuleHandle(nullptr);

    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = uniqueName.c_str();
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassEx(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        uniqueName.c_str(),
        title.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        size.GetX(),
        size.GetY(),
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

	return {};
}

types::boolean triton::editor::XWidgetBackendWin32::ProcessMessage()
{
    MSG msg;
    BOOL error = GetMessage(&msg, nullptr, 0, 0);
    if (error)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return error == TRUE ? K_TRUE : K_FALSE;
}