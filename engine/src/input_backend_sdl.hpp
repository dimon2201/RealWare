// input_backend_sdl.hpp

#pragma once

#include <SDL3/SDL.h>
#include <unordered_map>
#include "input_backend.hpp"
#include "input_key_state_enum.hpp"
#include "types.hpp"

namespace triton
{
	class cContext;
    
	class CInputBackendSDL final : public IInputBackend
	{
        std::unordered_map<types::u32, EKeyCode> _keyMap = {};
        EKeyState _keyState[256] = {};
        types::boolean _isFocused = types::False;
        types::boolean _bMouseMoved = types::False;
        cVector2 _cursorPosition = cVector2(0.0f);
        cVector2 _mouseDelta = cVector2(0.0f);

	public:
		explicit CInputBackendSDL(cContext* context);
		virtual ~CInputBackendSDL() override final = default;

        virtual SWindowBackend CreateBackendWindow(
            const std::string& title,
            const cVector2& size,
            types::boolean fullscreen
        ) override final;

        virtual void DestroyBackendWindow(SWindowBackend& window) override final;

        virtual void ResizeWindow(SWindowBackend& window, const cVector2& size) override final;

        std::vector<const char*> GetBackendWindowVulkanExtensions() override final;

        virtual void PreparePollEvent() override final;

        virtual SWindowEvent PollEvent() override final;

        virtual void ProcessEvent(const SWindowEvent& event, SWindowBackend& window) override final;

        virtual void* GetWindowWin32Handle(SWindowBackend& window) override final;

        virtual EKeyState GetKey(EKeyCode key) override final;

        virtual cVector2 GetCursorPosition(SWindowBackend& window) override final;

        virtual cVector2 GetMouseDelta() override final;

        virtual cVector2 GetMonitorSize() override final;

        virtual void SetKey(types::cpuword internalKey, EKeyState state) override final;

        virtual void SetWindowFocus(types::boolean isFocused) override final;

        virtual void SetWindowMouse(
            const cVector2& cursorPosition,
            const cVector2& mouseDelta
        ) override final;

        virtual void SetVSync(types::cpuword flag) override final;

        virtual types::boolean IsWindowFocused() override final;

    private:
        void InitializeKeyMap();
	};
}