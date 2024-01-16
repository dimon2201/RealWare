#pragma once

#include <windows.h>
#include "../../../engine/thirdparty/glm/glm/glm.hpp"

namespace realware
{
    namespace editor
    {
        class cEditorLabel
        {

        public:
            cEditorLabel(
                HWND parent,
                const std::string& text,
                const glm::vec2& position,
                const glm::vec2& size
            );
            ~cEditorLabel();

            inline HWND GetHWND() { return m_HWND; }

        private:
            HWND m_HWND = nullptr;

        };
    }
}