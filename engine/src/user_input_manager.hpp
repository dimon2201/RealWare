#pragma once

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <windows.h>

#include "../../thirdparty/glm/glm/glm.hpp"
#include "application.hpp"

namespace realware
{
    namespace core
    {
        class mUserInput
        {

        public:
            enum eButtonState
            {
                PRESSED = 0,
                RELEASED = 1,
                REPEAT = 2
            };

            mUserInput() {}
            ~mUserInput() {}

            void Init();
            void Free();

            void CreateAppWindow(const sApplicationDescriptor::sWindowDescriptor& desc);
            void Update();
            void PollEvents();
            void SwapBuffers();
            inline void FocusWindow() { glfwFocusWindow((GLFWwindow*)m_window); }

            boolean GetKey(int key);
            mUserInput::eButtonState GetMouseKey(int key);
            glm::vec2 GetMonitorSize();
            inline void* GetWindow() { return m_window; }
            inline glm::vec2 GetWindowSize() { return m_size; }
            inline const char* GetWindowTitle() { return m_desc.Title; }
            inline HWND GetWindowHWND() { return glfwGetWin32Window((GLFWwindow*)m_window); }
            inline boolean GetRunState() { return glfwWindowShouldClose((GLFWwindow*)m_window); }
            glm::vec2 GetCursorPosition();
            inline glm::vec2& GetCursorDelta() { return m_cursorDelta; }
            inline void SetWindowPosition(const glm::vec2& position) {
                glfwSetWindowPos((GLFWwindow*)m_window, position.x, position.y);
            }

        private:
            sApplicationDescriptor::sWindowDescriptor m_desc;
            void* m_window;
            glm::vec2 m_size;
            glm::vec2 m_cursorPosition;
            glm::vec2 m_prevCursorPosition;
            glm::vec2 m_cursorDelta;

        };
    }
}