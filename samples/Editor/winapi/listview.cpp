#include "window.hpp"
#include "listview.hpp"
#include "../../../engine/src/ecs.hpp"
#include "../editor_types.hpp"
#include "../../../engine/thirdparty/stb-master/stb_image.h"

#pragma comment(lib, "Comctl32.lib")

extern realware::core::cApplication* editorApp;
extern realware::core::cScene* editorScene;

extern eSelectMode editorSelectMode;
extern int editorSelectedAssetIndex;
extern int editorUsedAssetIndex;
extern realware::editor::cEditorWindow* editorWindowEntity;
extern realware::editor::cEditorWindow* editorWindowSound;
extern realware::editor::cEditorWindow* editorWindowScript;
extern realware::editor::cEditorListView* editorWindowAssetListView;
extern eAssetSelectedType editorWindowAssetSelectedType;
extern std::vector<std::vector<sAsset>> editorWindowAssetData;

extern void EditorWindowAssetShowPopupmenu(realware::core::boolean rmbPress);
extern void EditorWindowAssetDeleteItem(
    realware::core::cApplication* app,
    realware::core::cScene* scene,
    const eAssetSelectedType& type,
    int assetIndex
);
extern void EditorWindowEntityUpdate(int assetIndex);
extern void EditorWindowSoundUpdate(int assetIndex);
extern void EditorWindowScriptUpdate(int assetIndex);

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
                    sAsset asset("New", { "", "" });
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
                        EditorWindowAssetDeleteItem(
                            editorApp,
                            editorScene,
                            editorWindowAssetSelectedType,
                            index
                        );

                        editorWindowAssetData[editorWindowAssetSelectedType].erase(
                            editorWindowAssetData[editorWindowAssetSelectedType].begin() + index
                        );
                        editorWindowAssetListView->RemoveItem(0, index);
                    }
                }
                else if (LOWORD(wp) == 3)
                {
                    // Use
                    int index = editorWindowAssetListView->GetSelectedIndex();
                    if (index != -1)
                    {
                        editorSelectMode = eSelectMode::CREATE;
                        editorUsedAssetIndex = index;
                    }
                }
            }
            else if (wm == WM_LBUTTONDBLCLK)
            {
                // Show proper window
                int index = editorWindowAssetListView->GetSelectedIndex();
                if (index != -1)
                {
                    // Entity
                    if (editorWindowAssetSelectedType == eAssetSelectedType::ENTITY)
                    {
                        editorSelectedAssetIndex = index;
                        editorWindowEntity->Show(core::K_TRUE);
                        EditorWindowEntityUpdate(editorSelectedAssetIndex);
                    }
                    // Sound
                    else if (editorWindowAssetSelectedType == eAssetSelectedType::SOUND)
                    {
                        editorSelectedAssetIndex = index;
                        editorWindowSound->Show(core::K_TRUE);
                        EditorWindowSoundUpdate(editorSelectedAssetIndex);
                    }
                    // Script
                    else if (editorWindowAssetSelectedType == eAssetSelectedType::SCRIPT)
                    {
                        editorSelectedAssetIndex = index;
                        editorWindowScript->Show(core::K_TRUE);
                        EditorWindowScriptUpdate(editorSelectedAssetIndex);
                    }
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
            const glm::vec2& position,
            float monitorSizeCoef
        )
        {
            HINSTANCE hInstance = GetModuleHandle(0);

            RECT rcClient;
            GetClientRect(parent, &rcClient);
            m_HWND = CreateWindowEx(
                0,
                WC_LISTVIEW,
                windowName.data(),
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | LVS_SINGLESEL | LVS_REPORT | LVS_SMALLICON,
                position.x,
                position.y,
                rcClient.right - (monitorSizeCoef * 10.0f),
                rcClient.bottom - (monitorSizeCoef * 10.0f),
                parent,
                0,
                hInstance,
                0
            );

            SetWindowSubclass(m_HWND, ListViewProc, 0, 0);

            if (!m_HWND) {
                MessageBox(0, TEXT("Could not create listview"), 0, MB_ICONERROR);
                return;
            }

            core::u32* nullMem = new core::u32[32 * 32];
            memset(nullMem, 0, 32 * 32 * 4);
            int width0 = 0, height0 = 0, channels0 = 0,
                width1 = 0, height1 = 0, channels1 = 0,
                width2 = 0, height2 = 0, channels2 = 0;
            unsigned char* data0 = nullptr, *data1 = nullptr, *data2 = nullptr;
            data0 = stbi_load("icons/icon0.png", &width0, &height0, &channels0, 4);
            data1 = stbi_load("icons/icon1.png", &width1, &height1, &channels1, 4);
            data2 = stbi_load("icons/icon2.png", &width2, &height2, &channels2, 4);
            for (core::s32 i = 0; i < width0 * height0; i++)
            {
                core::u8 r = data0[i * 4];
                core::u8 g = data0[(i * 4) + 1];
                core::u8 b = data0[(i * 4) + 2];
                core::u8 a = data0[(i * 4) + 3];
                data0[(i * 4)] = b;
                data0[(i * 4) + 1] = g;
                data0[(i * 4) + 2] = r;
                data0[(i * 4) + 3] = a;
            }
            for (core::s32 i = 0; i < width1 * height1; i++)
            {
                core::u8 r = data1[i * 4];
                core::u8 g = data1[(i * 4) + 1];
                core::u8 b = data1[(i * 4) + 2];
                core::u8 a = data1[(i * 4) + 3];
                data1[(i * 4)] = b;
                data1[(i * 4) + 1] = g;
                data1[(i * 4) + 2] = r;
                data1[(i * 4) + 3] = a;
            }
            for (core::s32 i = 0; i < width2 * height2; i++)
            {
                core::u8 r = data2[i * 4];
                core::u8 g = data2[(i * 4) + 1];
                core::u8 b = data2[(i * 4) + 2];
                core::u8 a = data2[(i * 4) + 3];
                data2[(i * 4)] = b;
                data2[(i * 4) + 1] = g;
                data2[(i * 4) + 2] = r;
                data2[(i * 4) + 3] = a;
            }

            auto icon0 = CreateIcon(hInstance, 32, 32, 1, 32, (const BYTE*)nullMem, (const BYTE*)data0);
            auto icon1 = CreateIcon(hInstance, 32, 32, 1, 32, (const BYTE*)nullMem, (const BYTE*)data1);
            auto icon2 = CreateIcon(hInstance, 32, 32, 1, 32, (const BYTE*)nullMem, (const BYTE*)data2);
            auto imageList = ImageList_Create(32, 32, ILC_MASK, 3, 3);
            ImageList_AddIcon(imageList, icon0);
            ImageList_AddIcon(imageList, icon1);
            ImageList_AddIcon(imageList, icon2);
            ListView_SetImageList(m_HWND, imageList, LVSIL_SMALL);

            stbi_image_free(data0);
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
                itm.mask = LVIF_TEXT | LVIF_IMAGE;
                itm.iSubItem = 0;
                itm.iItem = index;
                if (editorWindowAssetSelectedType == eAssetSelectedType::ENTITY)
                {
                    itm.iImage = 0;
                }
                else if (editorWindowAssetSelectedType == eAssetSelectedType::SOUND)
                {
                    itm.iImage = 1;
                }
                else if (editorWindowAssetSelectedType == eAssetSelectedType::SCRIPT)
                {
                    itm.iImage = 2;
                }
                ListView_InsertItem(m_HWND, &itm);

            }
            else if (columnIndex > 0)
            {
                LVITEM itm = {};
                itm.pszText = LPSTR(name.data());
                itm.mask = LVIF_TEXT | LVIF_IMAGE;
                itm.iSubItem = columnIndex;
                itm.iItem = index;
                if (editorWindowAssetSelectedType == eAssetSelectedType::ENTITY)
                {
                    itm.iImage = 0;
                }
                else if (editorWindowAssetSelectedType == eAssetSelectedType::SOUND)
                {
                    itm.iImage = 1;
                }
                else if (editorWindowAssetSelectedType == eAssetSelectedType::SCRIPT)
                {
                    itm.iImage = 2;
                }
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