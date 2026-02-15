// input.hpp

#pragma once

#include "object.hpp"
#include "input_backend.hpp"
#include "math.hpp"
#include "types.hpp"

struct GLFWwindow;

namespace triton
{
	class cContext;

    class cInputWindow : public iObject
    {
        TRITON_OBJECT(cInputWindow)

        friend class cInput;

        sInputBackendWindow _backendWindow = {};

    public:
        enum class eRunState
        {
            OPENED = 1,
            CLOSED
        };

        explicit cInputWindow(cContext* context, const sInputBackendWindow& backendWindow);
        virtual ~cInputWindow() override final = default;

        void SwapBuffers();
        types::boolean IsWindowFocused() const;
        eRunState GetRunState();

        const cVector2& GetSize() const;
    };

	class cInput : public iObject
	{
        TRITON_OBJECT(cInput)

	public:
		explicit cInput(cContext* context);
		virtual ~cInput() override final = default;

        cInputWindow* CreatePlatformWindow(
            const std::string& title,
            const cVector2& size,
            types::boolean fullscreen
        );
        void DestroyWindow(cInputWindow* window);

        cVector2 GetMonitorSize() const;
        inline types::boolean GetKeyPressed(types::qword keyCode) const;
        inline types::boolean GetMouseKeyPressed(types::qword keyCode) const;
	};
}