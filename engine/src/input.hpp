// input.hpp

#pragma once

#include <string>
#include <windows.h>
#include "../../thirdparty/glm/glm/glm.hpp"
#include "object.hpp"
#include "types.hpp"

struct GLFWwindow;

namespace triton
{
	class cContext;

	class cInput : public iObject
	{
        TRITON_OBJECT(cInput)

        static constexpr types::usize kMaxKeyCount = 256;

        types::s32 _keys[kMaxKeyCount] = {};
        types::s32 _mouseKeys[3] = {};
        types::boolean _isFocused = types::K_FALSE;
        glm::vec2 _cursorPosition = glm::vec2(0.0f);

        friend class cWindow;
        friend void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
        friend void WindowFocusCallback(GLFWwindow* window, int focused);
        friend void WindowSizeCallback(GLFWwindow* window, int width, int height);
        friend void CursorCallback(GLFWwindow* window, double xpos, double ypos);
        friend void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

        inline void SetKey(const int key, types::boolean value) { _keys[key] = value; }
        inline void SetMouseKey(const int key, types::boolean value) { _mouseKeys[key] = value; }
        inline void SetWindowFocus(types::boolean value) { _isFocused = value; }
        inline void SetCursorPosition(const glm::vec2& cursorPosition) { _cursorPosition = cursorPosition; }

	public:
		explicit cInput(cContext* context);
		virtual ~cInput() override final = default;

        glm::vec2 GetMonitorSize() const;
        inline types::boolean GetKey(int key) const { return _keys[key]; }
        inline types::boolean GetMouseKey(int key) const { return _mouseKeys[key]; }
        inline types::boolean GetWindowFocus() const { return _isFocused; }
	};
}