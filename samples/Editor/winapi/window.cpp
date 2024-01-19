#include <string>
#include "window.hpp"
#include "listview.hpp"
#include "button.hpp"
#include "../editor_types.hpp"
#include "../../../engine/src/ecs.hpp"

using namespace realware::editor;

extern realware::core::cApplication* editorApp;
extern realware::core::cScene* editorScene;

extern std::vector<realware::render::sVertexBufferGeometry*> editorGeometriesToDraw;
extern int editorSelectedAssetIndex;
extern cEditorWindow* editorWindowAsset;
extern cEditorWindow* editorWindowEntity;
extern cEditorListView* editorWindowAssetListView;
extern cEditorButton* editorWindowAssetEntitiesButton;
extern cEditorButton* editorWindowAssetMaterialsButton;
extern cEditorButton* editorWindowEntityOKButton;
extern cEditorButton* editorWindowEntityCloseButton;
extern eAssetSelectedType editorWindowAssetSelectedType;
extern std::vector<std::vector<sAsset>> editorWindowAssetData;

extern void EditorWindowEntitySave(realware::core::cApplication* app, realware::core::cScene* scene, int assetIndex);

namespace realware
{
    namespace editor
    {
        LRESULT CALLBACK WndProc(HWND hwnd, UINT wm, WPARAM wp, LPARAM lp)
        {
            if (wm == WM_COMMAND)
            {
                // Entities
                if ((HWND)lp == editorWindowAssetEntitiesButton->GetHWND())
                {
                    editorWindowAssetSelectedType = eAssetSelectedType::ENTITY;
                    editorWindowAssetListView->AddItemsFromData(
                        editorWindowAssetData[editorWindowAssetSelectedType]
                    );
                }
                // Materials
                else if ((HWND)lp == editorWindowAssetMaterialsButton->GetHWND())
                {
                    editorWindowAssetSelectedType = eAssetSelectedType::MATERIAL;
                    editorWindowAssetListView->AddItemsFromData(
                        editorWindowAssetData[editorWindowAssetSelectedType]
                    );
                }
                // Entities OK
                else if ((HWND)lp == editorWindowEntityOKButton->GetHWND())
                {
                    editorWindowEntity->Show(core::K_FALSE);
                    EditorWindowEntitySave(editorApp, editorScene, editorSelectedAssetIndex);
                }
                // Entities Cancel
                else if ((HWND)lp == editorWindowEntityCloseButton->GetHWND())
                {
                    editorWindowEntity->Show(core::K_FALSE);
                }

                return 0;
            }

            return DefWindowProc(hwnd, wm, wp, lp);
        }

        cEditorWindow::cEditorWindow(
            HWND parent,
            const std::string& className,
            const std::string& windowName,
            const glm::vec2& position,
            const glm::vec2& size
        )
        {
            HINSTANCE hInstance = GetModuleHandle(0);

            WNDCLASSEX wc = {};
            MSG msg = {};

            wc.cbSize        = sizeof(wc);
            wc.style         = 0;
            wc.lpfnWndProc   = WndProc;
            wc.cbClsExtra    = 0;
            wc.cbWndExtra    = 0;
            wc.hInstance     = hInstance;
            wc.hIcon         = 0;
            wc.hCursor       = 0;
            wc.hbrBackground = 0;
            wc.lpszMenuName  = 0;
            wc.lpszClassName = className.data();
            wc.hIconSm       = 0;

            if (!RegisterClassEx(&wc)) {
                MessageBox(0, TEXT("Could not register window class"),
                           0, MB_ICONERROR);
                return;
            }
            
            m_HWND = CreateWindow(
                className.data(),
                windowName.data(),
                WS_OVERLAPPEDWINDOW,
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
                MessageBox(0, TEXT("Could not create window"), 0, MB_ICONERROR);
                return;
            }

            ShowWindow(m_HWND, 1);
            UpdateWindow(m_HWND);
        }

        cEditorWindow::~cEditorWindow()
        {
        }

        std::vector<HMENU> cEditorWindow::AddMenu(const std::vector<std::string>& popups)
        {
            auto hMenubar = CreateMenu();
            std::vector<HMENU> menus;
            for (auto& name : popups)
            {
                menus.emplace_back(CreateMenu());
                AppendMenuA(hMenubar, MF_POPUP, (UINT_PTR)menus.back(), name.data());
            }
            SetMenu(m_HWND, hMenubar);

            return menus;
        }

        void cEditorWindow::AddSubmenu(HMENU& parent, const std::string& name)
        {
            AppendMenuA(parent, MF_STRING, 0, name.data());
        }
    }
}