#pragma once

#include <windows.h>
#include "../../../engine/thirdparty/glm/glm/glm.hpp"
#include "../../../engine/src/types.hpp"

namespace realware
{
    namespace editor
    {
        class cEditorTextbox
        {

        public:
            cEditorTextbox(
                HWND parent,
                const std::string& windowName,
                const glm::vec2& position,
                const glm::vec2& size,
                core::boolean isNumberOnly,
                core::boolean isMultiline
            );
            ~cEditorTextbox();

            inline HWND GetHWND() { return m_HWND; }
            inline std::string GetText(core::s32 maxCharCount)
            {
                core::s8 data[256] = {};
                GetWindowText(m_HWND, (LPSTR)&data[0], maxCharCount);

                return std::string((const char*)&data[0]);
            }

            inline void SetText(const std::string& text) { SetWindowText(m_HWND, text.data()); }
            inline void SetReadonly(core::boolean value)
            {
                SetWindowLongPtr(
                    m_HWND,
                    GWL_STYLE,
                    WS_CHILD | WS_VISIBLE | (m_isNumberOnly == core::K_TRUE ? ES_NUMBER : 0) |
                    (m_isMultiline == core::K_TRUE ? WS_VSCROLL | ES_MULTILINE : 0) |
                    (value == core::K_TRUE ? ES_READONLY : 0)
                );
            }

        private:
            HWND m_HWND = nullptr;
            core::boolean m_isNumberOnly = core::K_FALSE;
            core::boolean m_isMultiline = core::K_FALSE;

        };
    }
}