#include <string>
#include "button.hpp"
#include "../editor_types.hpp"

namespace realware
{
    namespace editor
    {
        cEditorButton::cEditorButton(
            HWND parent,
            const std::string& windowName,
            const glm::vec2& position,
            const glm::vec2& size
        )
        {
            HINSTANCE hInstance = GetModuleHandle(0);

            m_HWND = CreateWindow(
                "BUTTON",
                windowName.data(),
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                position.x,
                position.y,
                size.x,
                size.y,
                parent,
                0,
                hInstance,
                0
            );

            if (!m_HWND)
            {
                MessageBox(0, TEXT("Could not create button"), 0, MB_ICONERROR);
                return;
            }

            ShowWindow(m_HWND, 1);
            UpdateWindow(m_HWND);
        }

        cEditorButton::~cEditorButton()
        {
        }
    }
}