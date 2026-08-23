// input_backend.hpp

#pragma once

#include <vector>
#include <string>
#include "math.hpp"
#include "object.hpp"
#include "backend.hpp"
#include "input_window_backend.hpp"
#include "input_window_event.hpp"
#include "input_key_code_enum.hpp"
#include "input_key_state_enum.hpp"
#include "types.hpp"

namespace triton
{
	class cContext;

	class IInputBackend : public iBackend
	{
        TRITON_CLASS_NAME(IInputBackend)

	public:
		explicit IInputBackend(cContext* context) : iBackend(context) {}
		~IInputBackend() override = default;

        virtual SWindowBackend CreateBackendWindow(
            const ::std::string& title,
            const cVector2& size,
            types::boolean fullscreen
        ) = 0;

        virtual void DestroyBackendWindow(SWindowBackend& window) = 0;

        virtual void ResizeWindow(SWindowBackend& window, const cVector2& size) = 0;

        virtual std::vector<const char*> GetBackendWindowVulkanExtensions() = 0;

        virtual void PreparePollEvent() = 0;

        virtual SWindowEvent PollEvent() = 0;

        virtual void ProcessEvent(const SWindowEvent& event, SWindowBackend& window) = 0;

        virtual void* GetWindowWin32Handle(SWindowBackend& window) = 0;

        virtual EKeyState GetKey(EKeyCode key) = 0;

        virtual cVector2 GetCursorPosition(SWindowBackend& window) = 0;

        virtual cVector2 GetMouseDelta() = 0;

        virtual cVector2 GetMonitorSize() = 0;

        virtual void SetKey(types::cpuword internalKey, EKeyState state) = 0;

        virtual void SetWindowFocus(types::boolean isFocused) = 0;

        virtual void SetWindowMouse(const cVector2& cursorPosition, const cVector2& mouseDelta) = 0;

        virtual void SetVSync(types::cpuword flag) = 0;

        virtual types::boolean IsWindowFocused() = 0;
	};
}