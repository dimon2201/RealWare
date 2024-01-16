#include <iostream>
#include <GLFW/glfw3.h>
#include "user_input_manager.hpp"
#include "render_manager.hpp"

extern realware::core::mUserInput* userInputManager;
extern realware::render::mRender* renderManager;

namespace realware
{
    namespace core
    {
        s32 g_keys[256] = {};
        boolean g_isFocused = K_FALSE;
        sApplicationDescriptor::sWindowDescriptor g_windowDesc;
        glm::vec2 g_cursorPosition = glm::vec2(0.0f);

        void ErrorCallback(int error_code, const char* description)
        {
            std::cout << error_code << " " <<  description << std::endl;
        }

        void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            if (action == GLFW_PRESS) {
                g_keys[key] = K_TRUE;
            } else if (action == GLFW_RELEASE) {
                g_keys[key] = K_FALSE;
            }
        }

        void FocusCallback(GLFWwindow* window, int focused)
        {
            if (focused)
            {
                if (g_isFocused == K_FALSE)
                {
                    g_isFocused = K_TRUE;

                    glm::vec2 size = userInputManager->GetMonitorSize();
                    g_windowDesc.Width = size.x;
                    g_windowDesc.Height = size.y;
                }
            }
            else
            {
                g_isFocused = K_FALSE;
            }
        }

        void CursorCallback(GLFWwindow* window, double xpos, double ypos)
        {
            g_cursorPosition.x = xpos;
            g_cursorPosition.y = ypos;
        }

        void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
        {
        }

        void WindowSizeCallback(GLFWwindow* window, int width, int height)
        {
            renderManager->ResizeWindow(glm::vec2(width, height));
        }

        void mUserInput::Init()
        {
        }

        void mUserInput::Free()
        {
        }

        void mUserInput::CreateAppWindow(const sApplicationDescriptor::sWindowDescriptor& desc)
        {
            m_desc = desc;
            g_windowDesc = m_desc;

            glfwInit();
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
            glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
            glfwSetErrorCallback(ErrorCallback);

            if (desc.IsFullscreen == K_FALSE)
            {
                m_window = glfwCreateWindow(m_desc.Width, m_desc.Height, m_desc.Title, nullptr, nullptr);
            }
            else
            {
                glfwWindowHint(GLFW_DECORATED, 0);

                glm::vec2 monitorSize = GetMonitorSize();
                m_desc.Width = monitorSize.x;
                m_desc.Height = monitorSize.y;
                m_window = glfwCreateWindow(monitorSize.x, monitorSize.y, m_desc.Title, glfwGetPrimaryMonitor(), nullptr);
            }

            glfwMakeContextCurrent((GLFWwindow*)m_window);
            glfwSwapInterval(1);
            glfwSetKeyCallback((GLFWwindow*)m_window, &KeyCallback);
            glfwSetWindowFocusCallback((GLFWwindow*)m_window, &FocusCallback);
            glfwSetCursorPosCallback((GLFWwindow*)m_window, &CursorCallback);
            glfwSetMouseButtonCallback((GLFWwindow*)m_window, &MouseButtonCallback);
            glfwSetWindowSizeCallback((GLFWwindow*)m_window, &WindowSizeCallback);

            m_size = glm::vec2(m_desc.Width, m_desc.Height);
        }

        void mUserInput::Update()
        {
            double x = 0.0, y = 0.0;
            glfwGetCursorPos((GLFWwindow*)m_window, &x, &y);
            m_prevCursorPosition = m_cursorPosition;
            m_cursorPosition = glm::vec2(x, y);
            m_cursorDelta = m_prevCursorPosition - m_cursorPosition;
        }

        void mUserInput::PollEvents()
        {
            glfwPollEvents();
        }

        void mUserInput::SwapBuffers()
        {
            glfwSwapBuffers((GLFWwindow*)m_window);
        }

        boolean mUserInput::GetKey(int key)
        {
            return g_keys[key];
        }

        mUserInput::eButtonState mUserInput::GetMouseKey(int key)
        {
            int state = glfwGetMouseButton((GLFWwindow*)m_window, key);

            if (state == GLFW_PRESS)
            {
                return eButtonState::PRESSED;
            }
            else if (state == GLFW_RELEASE)
            {
                return eButtonState::RELEASED;
            }

            return eButtonState::RELEASED;
        }

        glm::vec2 mUserInput::GetMonitorSize()
        {
            return glm::vec2(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
            //return glm::vec2(glfwGetVideoMode(glfwGetPrimaryMonitor())->width, glfwGetVideoMode(glfwGetPrimaryMonitor())->height);
        }

        glm::vec2 mUserInput::GetCursorPosition()
        {
            return glm::vec2(g_cursorPosition.x, g_cursorPosition.y);
        }
    }
}