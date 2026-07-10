// input_backend_sdl.hpp

#pragma once

#include "input_backend.hpp"
#include "types.hpp"

struct GLFWwindow;

namespace triton
{
	class cContext;
    
	class cInputBackendSDL final : public iInputBackend
	{
        TRITON_OBJECT(cInputBackendSDL)

        static constexpr types::usize kMaxKeyboardKeyCount = 256;
        static constexpr types::usize kMaxMouseKeyCount = 256;
        types::s32 _keys[kMaxKeyboardKeyCount] = {};
        types::s32 _mouseKeys[kMaxMouseKeyCount] = {};
        types::boolean _isFocused = types::K_FALSE;
        cVector2 _cursorPosition = cVector2(0.0f);

        static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void WindowFocusCallback(GLFWwindow* window, int focused);
        static void WindowSizeCallback(GLFWwindow* window, int width, int height);
        static void CursorCallback(GLFWwindow* window, double xpos, double ypos);
        static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

	public:
		explicit cInputBackendSDL(cContext* context);
		virtual ~cInputBackendSDL() override final = default;

        virtual sInputBackendWindow CreatePlatformWindow(
            const std::string& title,
            const cVector2& size,
            types::boolean fullscreen
        ) override final;
        virtual void DestroyWindow(sInputBackendWindow& window) override final;
        virtual void ResizeWindow(sInputBackendWindow& window, const cVector2& size) override final;
        virtual SEvent PollEvent() override final;
        virtual void ProcessEvent(const SEvent& event) override final;
        virtual void* GetWindowWin32Handle(sInputBackendWindow& window) override final;
        virtual types::boolean GetKeyPressed(types::qword keyCode) override final;
        virtual types::boolean GetMouseKeyPressed(types::qword keyCode) override final;
        virtual cVector2 GetCursorPosition(sInputBackendWindow& window) override final;
        virtual void SetKeyPressed(types::qword keyCode, types::boolean isPressed) override final;
        virtual void SetMouseKeyPressed(types::qword keyCode, types::boolean isPressed) override final;
        virtual void SetWindowFocus(types::boolean isFocused) override final;
        virtual void SetWindowCursorPosition(const cVector2& cursorPosition) override final;
        virtual void SetVSync(types::cpuword flag) override final;
        virtual cVector2 GetMonitorSize() override final;
        virtual types::boolean IsWindowFocused() override final;
	};
}