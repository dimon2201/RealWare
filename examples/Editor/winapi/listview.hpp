#pragma once

#include <string>
#include <windows.h>
#include "../../../engine/src/types.hpp"
#include "../../../engine/thirdparty/glm/glm/glm.hpp"
#include "../editor_types.hpp"
#include <commctrl.h>

namespace realware
{
    namespace editor
    {
        class cEditorListView
        {

        public:
            cEditorListView(
                HWND parent,
                const std::string& className,
                const std::string& windowName,
                const glm::vec2& position,
                float monitorSizeCoef
            );
            ~cEditorListView();

            void AddColumn(types::s32 index, const std::string& name, types::usize width);
            void AddItem(types::s32 columnIndex, types::s32 index, const std::string& name);
            void RemoveItem(types::s32 columnIndex, types::s32 index);
            void AddItemsFromData(const std::vector<sAsset>& data);

            inline HWND GetHWND() { return m_HWND; }
            inline int GetSelectedIndex() {
                return ListView_GetNextItem(m_HWND, -1, LVNI_SELECTED);
            }
            inline void SetSelected(int idx) {
                ListView_SetItemState(m_HWND, idx, 0xF, LVIS_SELECTED);
            }

        private:
            HWND m_HWND = nullptr;

        };
    }
}