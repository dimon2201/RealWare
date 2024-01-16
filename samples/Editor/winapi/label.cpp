#include <string>

#include "label.hpp"

namespace realware
{
    namespace editor
    {
        cEditorLabel::cEditorLabel(
            HWND parent,
            const std::string& text,
            const glm::vec2& position,
            const glm::vec2& size
        )
        {
            HINSTANCE hInstance = GetModuleHandle(0);

            m_HWND = CreateWindow(
                "STATIC",
                text.data(),
                WS_CHILD | WS_VISIBLE,
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
                MessageBox(0, TEXT("Could not create groupbox"), 0, MB_ICONERROR);
                return;
            }

            ShowWindow(m_HWND, 1);
            UpdateWindow(m_HWND);
        }

        cEditorLabel::~cEditorLabel()
        {
        }
    }
}