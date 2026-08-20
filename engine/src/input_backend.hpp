// input_backend.hpp

#pragma once

#include <string>
#include "math.hpp"
#include "object.hpp"
#include "backend.hpp"
#include "input_backend_window.hpp"
#include "input_window.hpp"
#include "math.hpp"
#include "types.hpp"

namespace triton
{
	class cContext;
    
    enum class EWindowEvent
    {
        None,
        KeyDown,
        KeyUp,
        FocusGained,
        FocusLost,
        Resized,
        MouseMotion,
        MouseButtonDown,
        MouseButtonUp,
        Quit
    };

    struct SEvent
    {
        EWindowEvent type = EWindowEvent::None;
        types::cpuword argA = 0;
        types::cpuword argB = 0;
    };

	class iInputBackend : public iBackend
	{
        TRITON_CLASS_NAME(iInputBackend)

	public:
		explicit iInputBackend(cContext* context);
		~iInputBackend() override = default;

        virtual sInputBackendWindow CreatePlatformWindow(
            const ::std::string& title,
            const cVector2& size,
            types::boolean fullscreen
        ) = 0;
        virtual void DestroyWindow(sInputBackendWindow& window) = 0;
        virtual void ResizeWindow(sInputBackendWindow& window, const cVector2& size) = 0;
        virtual SEvent PollEvent() = 0;
        virtual void ProcessEvent(const SEvent& event) = 0;
        virtual void* GetWindowWin32Handle(sInputBackendWindow& window) = 0;
        virtual types::boolean GetKeyPressed(types::qword keyCode) = 0;
        virtual types::boolean GetMouseKeyPressed(types::qword keyCode) = 0;
        virtual cVector2 GetCursorPosition(sInputBackendWindow& window) = 0;
        virtual void SetKeyPressed(types::qword keyCode, types::boolean isPressed) = 0;
        virtual void SetMouseKeyPressed(types::qword keyCode, types::boolean isPressed) = 0;
        virtual void SetWindowFocus(types::boolean isFocused) = 0;
        virtual void SetWindowCursorPosition(const cVector2& cursorPosition) = 0;
        virtual void SetVSync(types::cpuword flag) = 0;
        virtual cVector2 GetMonitorSize() = 0;
        virtual types::boolean IsWindowFocused() = 0;
        virtual types::cpuword GetKeyW() = 0;
        virtual types::cpuword GetKeyA() = 0;
        virtual types::cpuword GetKeyS() = 0;
        virtual types::cpuword GetKeyD() = 0;
	};
}