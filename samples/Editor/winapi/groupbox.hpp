#pragma once

#include <windows.h>
#include "../../../engine/thirdparty/glm/glm/glm.hpp"

namespace realware
{
    namespace editor
    {
        class cEditorGroupbox
        {

        public:
            cEditorGroupbox(
                HWND parent,
                const std::string& windowName,
                const glm::vec2& position,
                const glm::vec2& size
            );
            ~cEditorGroupbox();

            inline HWND GetHWND() { return m_HWND; }

        private:
            HWND m_HWND = nullptr;

        };
    }
}