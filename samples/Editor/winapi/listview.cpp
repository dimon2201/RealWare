#include "window.hpp"
#include "listview.hpp"
#include "../../../engine/src/ecs.hpp"
#include "../editor_types.hpp"

#pragma comment(lib, "Comctl32.lib")

extern int editorSelectedAssetIndex;
extern realware::editor::cEditorWindow* editorWindowEntity;
extern realware::editor::cEditorListView* editorWindowAssetListView;
extern eAssetSelectedType editorWindowAssetSelectedType;
extern std::vector<std::vector<sAsset>> editorWindowAssetData;

extern void EditorWindowAssetShowPopupmenu(realware::core::boolean rmbPress);
extern void EditorWindowEntityUpdate(int assetIndex);

namespace realware
{
    namespace editor
    {
        LRESULT CALLBACK ListViewProc(HWND hwnd, UINT wm, WPARAM wp, LPARAM lp, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
        {
            if (wm == WM_COMMAND)
            {
                if (LOWORD(wp) == 1)
                {
                    // New
                    sAsset asset("New");
                    editorWindowAssetData[editorWindowAssetSelectedType].push_back(asset);
                    editorWindowAssetListView->AddItem(
                        0,
                        editorWindowAssetData[editorWindowAssetSelectedType].size(),
                        asset.Name
                    );
                }
                else if (LOWORD(wp) == 2)
                {
                    // Delete
                    int index = editorWindowAssetListView->GetSelectedIndex();
                    if (index != -1)
                    {
                        editorWindowAssetData[editorWindowAssetSelectedType].erase(
                            editorWindowAssetData[editorWindowAssetSelectedType].begin() + index
                        );
                        editorWindowAssetListView->RemoveItem(0, index);
                    }
                }
            }
            else if (wm == WM_LBUTTONDBLCLK)
            {
                // Show entity window
                int index = editorWindowAssetListView->GetSelectedIndex();
                if (index != -1)
                {
                    editorSelectedAssetIndex = index;
                    editorWindowEntity->Show(core::K_TRUE);
                    EditorWindowEntityUpdate(editorSelectedAssetIndex);
                }
            }
            else if (wm == WM_RBUTTONDOWN)
            {
                // Popup menu
                EditorWindowAssetShowPopupmenu(realware::core::K_TRUE);
            }

            return DefSubclassProc(hwnd, wm, wp, lp);
        }

        cEditorListView::cEditorListView(
            HWND parent,
            const std::string& className,
            const std::string& windowName,
            const glm::vec2& position
        )
        {
            RECT rcClient;

            GetClientRect(parent, &rcClient);
            m_HWND = CreateWindowEx(
                0,
                WC_LISTVIEW,
                windowName.data(),
                WS_TABSTOP | WS_VISIBLE | WS_CHILD |
                LVS_SINGLESEL | LVS_REPORT | LVS_EDITLABELS,
                position.x,
                position.y,
                rcClient.right,
                rcClient.bottom,
                parent,
                0,
                GetModuleHandle(0),
                0
            );

            SetWindowSubclass(m_HWND, ListViewProc, 0, 0);

            if (!m_HWND) {
                MessageBox(0, TEXT("Could not create listview"), 0, MB_ICONERROR);
                return;
            }
        }

        cEditorListView::~cEditorListView()
        {
        }

        void cEditorListView::AddColumn(core::s32 index, const std::string& name, core::usize width)
        {
            LVCOLUMN col = {};
            col.pszText = LPSTR(name.data());
            col.mask = LVCF_TEXT;
            col.fmt = LVCFMT_LEFT;
            ListView_InsertColumn(m_HWND, index, &col);
            ListView_SetColumnWidth(m_HWND, index, width);
        }

        void cEditorListView::AddItem(core::s32 columnIndex, core::s32 index, const std::string& name)
        {
            if (columnIndex == 0)
            {
                LVITEM itm = {};
                itm.pszText = LPSTR(name.data());
                itm.mask = LVIF_TEXT;
                itm.iSubItem = 0;
                itm.iItem = index;
                ListView_InsertItem(m_HWND, &itm);

            }
            else if (columnIndex > 0)
            {
                LVITEM itm = {};
                itm.pszText = LPSTR(name.data());
                itm.mask = LVIF_TEXT;
                itm.iSubItem = columnIndex;
                itm.iItem = index;
                ListView_SetItem(m_HWND, &itm);
            }
        }

        void cEditorListView::RemoveItem(core::s32 columnIndex, core::s32 index)
        {
            ListView_DeleteItem(m_HWND, index);
        }

        void cEditorListView::AddItemsFromData(const std::vector<sAsset>& data)
        {
            ListView_DeleteAllItems(m_HWND);
            
            core::s32 index = 0;
            for (auto asset : data)
            {
                AddItem(0, index, asset.Name.data());
                index += 1;
            }
        }
    }
}