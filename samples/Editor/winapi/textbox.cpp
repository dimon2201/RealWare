#include <string>
#include "textbox.hpp"
#include "../editor_types.hpp"

namespace realware
{
    namespace editor
    {
        cEditorTextbox::cEditorTextbox(
            HWND parent,
            const std::string& windowName,
            const glm::vec2& position,
            const glm::vec2& size,
            core::boolean isNumberOnly,
            core::boolean isMultiline
        )
        {
            HINSTANCE hInstance = GetModuleHandle(0);

            m_HWND = CreateWindowEx(
                WS_EX_CLIENTEDGE,
                "EDIT",
                windowName.data(),
                WS_CHILD | WS_VISIBLE | (isNumberOnly == core::K_TRUE ? ES_NUMBER : 0) |
                (isMultiline == core::K_TRUE ? WS_VSCROLL | ES_MULTILINE : 0),
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
                MessageBox(0, TEXT("Could not create textbox"), 0, MB_ICONERROR);
                return;
            }

            ShowWindow(m_HWND, 1);
            UpdateWindow(m_HWND);
        }

        cEditorTextbox::~cEditorTextbox()
        {
        }
    }
}