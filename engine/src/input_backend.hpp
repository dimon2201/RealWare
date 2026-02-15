// input_backend.hpp

#pragma once

#include <string>
#include <windows.h>
#include "../../thirdparty/glm/glm/glm.hpp"
#include "backend.hpp"
#include "input.hpp"
#include "math.hpp"
#include "types.hpp"

namespace triton
{
	class cContext;
    
    struct sInputBackendWindow
    {
        types::qword instance = 0;
        ::std::string title = "";
        cVector2 size = cVector2(0.0f);
        types::boolean fullscreen = types::K_FALSE;
    };

	class iInputBackend : public iBackend
	{
        TRITON_OBJECT(iInputBackend)

	public:
		explicit iInputBackend(cContext* context);
		virtual ~iInputBackend() override = default;

        virtual sInputBackendWindow CreatePlatformWindow(
            const ::std::string& title,
            const cVector2& size,
            types::boolean fullscreen
        ) = 0;
        virtual void DestroyWindow(sInputBackendWindow& window) = 0;
        virtual void ResizeWindow(sInputBackendWindow& window, const cVector2& size) = 0;
        virtual void SwapWindowBuffers(sInputBackendWindow& window) = 0;
        virtual void PollEvents() = 0;
        virtual void* GetWindowWin32Handle(sInputBackendWindow& window) = 0;
        virtual triton::cInputWindow::eRunState GetWindowRunState(sInputBackendWindow& window) = 0;
        virtual types::boolean GetKeyPressed(types::qword keyCode) = 0;
        virtual types::boolean GetMouseKeyPressed(types::qword keyCode) = 0;
        virtual void SetKeyPressed(types::qword keyCode, types::boolean isPressed) = 0;
        virtual void SetMouseKeyPressed(types::qword keyCode, types::boolean isPressed) = 0;
        virtual void SetWindowFocus(types::boolean isFocused) = 0;
        virtual void SetWindowCursorPosition(const cVector2& cursorPosition) = 0;
        virtual cVector2 GetMonitorSize() = 0;
        virtual types::boolean IsWindowFocused() = 0;
	};
}