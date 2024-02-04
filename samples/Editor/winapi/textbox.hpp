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

        private:
            HWND m_HWND = nullptr;

        };
    }
}