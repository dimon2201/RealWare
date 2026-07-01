// input.hpp

#pragma once

#include "object.hpp"
#include "input_backend.hpp"
#include "input_window.hpp"
#include "math.hpp"
#include "types.hpp"

struct GLFWwindow;

namespace triton
{
	class cContext;
    template <typename TValue>
    class cStack;

	class cInput : public iObject
	{
        TRITON_OBJECT(cInput)

        cStack<cInputWindow>* _windows = nullptr;

	public:
		explicit cInput(cContext* context);
		virtual ~cInput() override final = default;

        void Initialize();
        void Shutdown();
        cInputWindow* CreatePlatformWindow(
            const std::string& title,
            const cVector2& size,
            types::boolean fullscreen
        );
        void DestroyWindow(cInputWindow* window);
        void ResizeWindows(const cVector2& newSize);
        cVector2 GetCursorPosition(cInputWindow* window);

        cVector2 GetMonitorSize() const;
        types::boolean GetKeyPressed(types::qword keyCode) const;
        inline types::boolean GetMouseKeyPressed(types::qword keyCode) const;
        inline cStack<cInputWindow>* GetWindows() const { return _windows; }
	};
}