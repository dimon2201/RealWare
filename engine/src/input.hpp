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