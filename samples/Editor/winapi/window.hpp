#pragma once

#include <vector>
#include <string>
#include <windows.h>
#include "../../../engine/src/types.hpp"
#include "../../../engine/thirdparty/glm/glm/glm.hpp"

namespace realware
{
    namespace editor
    {
        static inline void SetWindowTopMost(HWND hwnd)
        {
            SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        }

        static inline void SetWindowParent(HWND child, HWND parent)
        {
            SetParent(child, parent);
        }

        static inline void RemoveWindowSysmenu(HWND hwnd)
        {
            long Style = GetWindowLong(hwnd, GWL_STYLE);
            Style &= ~WS_SYSMENU;
            Style &= ~WS_MINIMIZEBOX;
            Style &= ~WS_MAXIMIZEBOX;
            Style &= ~WS_SIZEBOX;
            SetWindowLong(hwnd, GWL_STYLE, Style);
        }

        class cEditorWindow
        {

        public:
            cEditorWindow(
                HWND parent,
                const std::string& className,
                const std::string& windowName,
                const glm::vec2& position,
                const glm::vec2& size
            );
            ~cEditorWindow();

            std::vector<HMENU> AddMenu(const std::vector<std::string>& popups);
            void AddSubmenu(HMENU& parent, const std::string& name);
            inline void Show(core::boolean isVisible)
            {
                if (isVisible == core::K_TRUE) {
                    ShowWindow(m_HWND, 1);
                } else if (isVisible == core::K_FALSE) {
                    ShowWindow(m_HWND, SW_HIDE);
                }
            }

            inline HWND GetHWND() { return m_HWND; }

        private:
            HWND m_HWND = nullptr;

        };
    }
}