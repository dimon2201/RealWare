#include <iostream>
#include <chrono>
#include <ctime>
#include <vector>
#include <windows.h>

#include "../../engine/thirdparty/glm/glm/glm.hpp"
#include "../../engine/thirdparty/glm/glm/gtc/matrix_transform.hpp"
#include "../../engine/src/application.hpp"
#include "../../engine/src/ecs.hpp"
#include "../../engine/src/user_input_manager.hpp"
#include "../../engine/src/camera_manager.hpp"
#include "../../engine/src/texture_manager.hpp"
#include "../../engine/src/filesystem_manager.hpp"
#include "../../engine/src/render_manager.hpp"
#include "../../engine/src/physics_manager.hpp"
#include "../../engine/src/render_context.hpp"
#include "../../engine/src/sound_context.hpp"
#include "../../engine/src/sound_manager.hpp"
#include "../../engine/src/font_manager.hpp"
#include "../../engine/src/widget_manager.hpp"
#include "../../engine/src/types.hpp"
#include "winapi/window.hpp"
#include "winapi/listview.hpp"
#include "winapi/groupbox.hpp"
#include "winapi/textbox.hpp"
#include "winapi/label.hpp"
#include "winapi/button.hpp"
#include "ray_hit.hpp"
#include "editor_types.hpp"

using namespace realware::core;
using namespace realware::render;
using namespace realware::font;
using namespace realware::sound;
using namespace realware::physics;
using namespace realware::editor;

mRender* renderManager = new mRender();
mTexture* textureManager = new mTexture();
mCamera* cameraManager = new mCamera();
mFileSystem* fileSystemManager = new mFileSystem();
mUserInput* userInputManager = new mUserInput();
mFont* fontManager = new mFont();
mSound* soundManager = new mSound();
mPhysics* physicsManager = new mPhysics();

eSelectMode editorSelectMode = eSelectMode::NONE;
entity editorSelectedEntity = 0;
int editorSelectedAssetIndex = 0;
glm::vec3 editorPosition = glm::vec3(0.0f), editorRotation = glm::vec3(0.0f), editorScale = glm::vec3(0.0f);
sTextboxLabel editorPositionX; sTextboxLabel editorPositionY; sTextboxLabel editorPositionZ;
sTextboxLabel editorRotationX; sTextboxLabel editorRotationY; sTextboxLabel editorRotationZ;
sTextboxLabel editorScaleX; sTextboxLabel editorScaleY; sTextboxLabel editorScaleZ;
sTextboxLabel editorWindowEntityName;
cEditorWindow* editorWindowAsset = nullptr;
cEditorWindow* editorWindowEntity = nullptr;
cEditorListView* editorWindowAssetListView = nullptr;
cEditorButton* editorWindowAssetEntitiesButton = nullptr;
cEditorButton* editorWindowAssetMaterialsButton = nullptr;
cEditorButton* editorWindowEntityOKButton = nullptr;
cEditorButton* editorWindowEntityCloseButton = nullptr;
eAssetSelectedType editorWindowAssetSelectedType = eAssetSelectedType::ENTITY;
std::vector<std::vector<sAsset>> editorWindowAssetData((int)eAssetSelectedType::_COUNT);

void EditorUpdateTextboxTransform(sCTransform* transform);
void EditorUpdateEntityTransform(cScene* scene);
void EditorWindowAssetShowPopupmenu(realware::core::boolean rmbPress);
void EditorWindowEntityUpdate(int assetIndex);
void EditorWindowEntitySave(int assetIndex);

class MyApp : public cApplication
{

public:
    MyApp(const sApplicationDescriptor& desc) : cApplication(desc) {}
    ~MyApp() {}

    virtual void Init() override final
    {
        // Initialize managers
        textureManager->Init(m_renderContext, 1920, 1080, 12);
        renderManager->Init(m_renderContext, 65536, 65536, 65536, 65536, glm::vec2(m_desc.WindowDesc.Width, m_desc.WindowDesc.Height));
        cameraManager->Init();
        userInputManager->Init();
        physicsManager->Init();

        // Triangle primitive
        auto plane = renderManager->CreateModel("data/models/plane.fbx");
        m_geomPlane = renderManager->AddGeometry(
            plane->Format,
            plane->VerticesByteSize,
            plane->Vertices,
            plane->IndicesByteSize,
            plane->Indices
        );

        auto taburet = renderManager->CreateModel("data/models/taburet2.fbx");
        m_geomTaburet = renderManager->AddGeometry(
            taburet->Format,
            taburet->VerticesByteSize,
            taburet->Vertices,
            taburet->IndicesByteSize,
            taburet->Indices
        );

        auto dirtTexture = textureManager->CreateTexture("data/textures/dirt.png", "DirtTexture");
        m_taburetTexture = textureManager->CreateTexture("data/textures/taburet2.png", "TaburetTexture");

        // Scene
        m_scene = new cScene(65536);

        // Physics scene
        entity physicsSceneEntity = m_scene->CreateEntity("PhysicsSceneEntity");
        physicsManager->AddScene({ physicsSceneEntity, m_scene });

        // Plane entity
        m_plane = m_scene->CreateEntity("PlaneEntity");
        sCGeometryInfo* planegi = m_scene->Add<sCGeometryInfo>(m_plane);
        planegi->IsVisible = K_TRUE;
        planegi->IsOpaque = K_TRUE;
        sCGeometry* planeg = m_scene->Add<sCGeometry>(m_plane);
        planeg->Geometry = m_geomPlane;
        sCTransform* planet = m_scene->Add<sCTransform>(m_plane);
        planet->Position = glm::vec3(0.0f);
        planet->Rotation = glm::vec3(0.0f);
        planet->Scale = glm::vec3(150.0f);
        sCMaterial* planem = m_scene->Add<sCMaterial>(m_plane);
        planem->DiffuseTexture = dirtTexture;
        planem->DiffuseColor = glm::vec4(1.0f);

        physicsManager->AddActor(
            { physicsSceneEntity, m_scene },
            { m_plane, m_scene },
            mPhysics::eActorDescriptor::STATIC,
            mPhysics::eShapeDescriptor::PLANE,
            glm::vec4(0.0f),
            nullptr
        );

        // Camera entity
        m_camera = m_scene->CreateEntity("CameraEntity");
        sCCamera* cameraCamera = m_scene->Add<sCCamera>(m_camera);
        cameraCamera->FOV = 65.0f;
        cameraCamera->ZNear = 0.01f;
        cameraCamera->ZFar = 100.0f;
        sCTransform* cameraTransform = m_scene->Add<sCTransform>(m_camera);
        cameraTransform->Position = glm::vec3(0.0f, 5.0f, 0.0f);

        sCPhysicsCharacterController* controller = physicsManager->AddCharacterController(
            { physicsSceneEntity, m_scene },
            { m_camera, m_scene },
            mPhysics::eShapeDescriptor::CAPSULE,
            glm::vec4(1.0f, 2.0f, 0.0f, 0.0f)
        );
        controller->IsGravityEnabled = K_FALSE;

        // Editor windows
        // Main window
        float offset = userInputManager->GetMonitorSize().x * 0.0025f;
        glm::vec2 mainWindowSize = glm::vec2(
                (userInputManager->GetMonitorSize().x / 2.0f) - (userInputManager->GetMonitorSize().x * 0.025f) - (userInputManager->GetMonitorSize().x * 0.0125f),
                (userInputManager->GetMonitorSize().y * 0.142f)
        );
        auto mainWindow = new cEditorWindow(
            nullptr,
            "MainWindow",
            "RealWare Editor",
            glm::vec2(0.0f),
            mainWindowSize
        );
        auto menus = mainWindow->AddMenu({ "Plugin" });
        mainWindow->AddSubmenu(menus[0], "New");
        mainWindow->AddSubmenu(menus[0], "Open");
        mainWindow->AddSubmenu(menus[0], "Save");

        // Asset window
        glm::vec2 assetWindowSize = glm::vec2(
            (userInputManager->GetMonitorSize().x / 2.0f) - (userInputManager->GetMonitorSize().x * 0.025f) - (userInputManager->GetMonitorSize().x * 0.0125f),
            userInputManager->GetMonitorSize().y - (userInputManager->GetMonitorSize().y * 0.2f)
        );
        editorWindowAsset = new cEditorWindow(
            nullptr,
            "AssetWindow",
            "Assets",
            glm::vec2(0.0f, (userInputManager->GetMonitorSize().y * 0.1f) + (userInputManager->GetMonitorSize().y * 0.05f)),
            assetWindowSize
        );
        RemoveWindowSysmenu(editorWindowAsset->GetHWND());
        editorWindowAssetListView = new cEditorListView(
            editorWindowAsset->GetHWND(),
            "AssetListView",
            "Asset view",
            glm::vec2(0.0f, offset * 20.0f)
        );
        editorWindowAssetListView->AddColumn(0, "Name", 150);

        editorWindowAssetEntitiesButton = new cEditorButton(
            editorWindowAsset->GetHWND(),
            "Entities",
            glm::vec2(offset * 2.0f),
            glm::vec2(offset * 25.0f, offset * 7.0f)
        );
        editorWindowAssetMaterialsButton = new cEditorButton(
            editorWindowAsset->GetHWND(),
            "Materials",
            glm::vec2(offset * 28.0f, offset * 2.0f),
            glm::vec2(offset * 25.0f, offset * 7.0f)
        );

        // Entity window
        editorWindowEntity = new cEditorWindow(
            nullptr,
            "EntityWindow",
            "Entity",
            glm::vec2(0.0f),
            glm::vec2(offset * 120.0f, offset * 80.0f)
        );
        editorWindowEntity->Show(K_FALSE);
        RemoveWindowSysmenu(editorWindowEntity->GetHWND());

        editorWindowEntityOKButton = new cEditorButton(
            editorWindowEntity->GetHWND(),
            "OK",
            glm::vec2(offset * 106.0f, offset * 60.0f),
            glm::vec2(offset * 7.0f, offset * 7.0f)
        );
        editorWindowEntityCloseButton = new cEditorButton(
            editorWindowEntity->GetHWND(),
            "Close",
            glm::vec2(offset * 90.0f, offset * 60.0f),
            glm::vec2(offset * 15.0f, offset * 7.0f)
        );
        editorWindowEntityName.Label = new cEditorLabel(editorWindowEntity->GetHWND(), "Name",
            glm::vec2(offset * 3.0f, offset * 5.0f), glm::vec2(offset * 13.0f, offset * 7.0f)
        );
        editorWindowEntityName.Textbox = new cEditorTextbox(editorWindowEntity->GetHWND(), "",
            glm::vec2(offset * 20.0f, offset * 5.0f), glm::vec2(offset * 90.0f, offset * 7.0f), K_FALSE
        );

        // Render window
        userInputManager->SetWindowPosition(glm::vec2(assetWindowSize.x + (userInputManager->GetMonitorSize().x * 0.006f), userInputManager->GetMonitorSize().y * 0.041f));
        RemoveWindowSysmenu(userInputManager->GetWindowHWND());

        // Object window
        glm::vec2 objectWindowSize = glm::vec2(
            userInputManager->GetMonitorSize().x / 1.86f, userInputManager->GetMonitorSize().y / 3.91f
        );
        auto objectWindow = new cEditorWindow(
            nullptr,
            "ObjectWindow",
            "Object",
            glm::vec2(assetWindowSize.x, (userInputManager->GetMonitorSize().y * 0.016f) + (userInputManager->GetMonitorSize().y * 0.091f) + (userInputManager->GetMonitorSize().y / 1.7f)),
            objectWindowSize
        );
        RemoveWindowSysmenu(objectWindow->GetHWND());

        auto objectPositionGroupbox = new cEditorGroupbox(
            objectWindow->GetHWND(),
            "Position",
            glm::vec2(offset),
            glm::vec2(objectWindowSize.x - offset * 140.0f, objectWindowSize.y - offset * 45.0f)
        );
        auto objectRotationGroupbox = new cEditorGroupbox(
            objectWindow->GetHWND(),
            "Rotation",
            glm::vec2(offset, offset * 15.0f),
            glm::vec2(objectWindowSize.x - offset * 140.0f, objectWindowSize.y - offset * 45.0f)
        );
        auto objectScaleGroupbox = new cEditorGroupbox(
            objectWindow->GetHWND(),
            "Scale",
            glm::vec2(offset, offset * 30.0f),
            glm::vec2(objectWindowSize.x - offset * 140.0f, objectWindowSize.y - offset * 45.0f)
        );
        auto objectComponentsGroupbox = new cEditorGroupbox(
            objectWindow->GetHWND(),
            "Components",
            glm::vec2((offset * 2.0f) + (objectWindowSize.x - (offset * 140.0f)), offset),
            glm::vec2(objectWindowSize.x - offset * 82.0f, objectWindowSize.y - offset * 13.0f)
        );

        editorPositionX.Label = new cEditorLabel(objectPositionGroupbox->GetHWND(), "X",
            glm::vec2(offset * 3.0f, offset * 5.0f), glm::vec2(offset * 3.0f, offset * 5.0f)
        );
        editorPositionX.Textbox = new cEditorTextbox(objectPositionGroupbox->GetHWND(), "0",
            glm::vec2(offset * 8.0f, offset * 5.0f), glm::vec2(offset * 18.0f, offset * 5.0f), K_TRUE
        );
        editorPositionY.Label = new cEditorLabel(objectPositionGroupbox->GetHWND(), "Y",
            glm::vec2(offset * 26.0f, offset * 5.0f), glm::vec2(offset * 3.0f, offset * 5.0f)
        );
        editorPositionY.Textbox = new cEditorTextbox(objectPositionGroupbox->GetHWND(), "0",
            glm::vec2(offset * 31.0f, offset * 5.0f), glm::vec2(offset * 18.0f, offset * 5.0f), K_TRUE
        );
        editorPositionZ.Label = new cEditorLabel(objectPositionGroupbox->GetHWND(), "Z",
            glm::vec2(offset * 49.0f, offset * 5.0f), glm::vec2(offset * 3.0f, offset * 5.0f)
        );
        editorPositionZ.Textbox = new cEditorTextbox(objectPositionGroupbox->GetHWND(), "0",
            glm::vec2(offset * 54.0f, offset * 5.0f), glm::vec2(offset * 18.0f, offset * 5.0f), K_TRUE
        );
        editorRotationX.Label = new cEditorLabel(objectRotationGroupbox->GetHWND(), "X",
            glm::vec2(offset * 3.0f, offset * 5.0f), glm::vec2(offset * 3.0f, offset * 5.0f)
        );
        editorRotationX.Textbox = new cEditorTextbox(objectRotationGroupbox->GetHWND(), "0",
            glm::vec2(offset * 8.0f, offset * 5.0f), glm::vec2(offset * 18.0f, offset * 5.0f), K_TRUE
        );
        editorRotationY.Label = new cEditorLabel(objectRotationGroupbox->GetHWND(), "Y",
            glm::vec2(offset * 26.0f, offset * 5.0f), glm::vec2(offset * 3.0f, offset * 5.0f)
        );
        editorRotationY.Textbox = new cEditorTextbox(objectRotationGroupbox->GetHWND(), "0",
            glm::vec2(offset * 31.0f, offset * 5.0f), glm::vec2(offset * 18.0f, offset * 5.0f), K_TRUE
        );
        editorRotationZ.Label = new cEditorLabel(objectRotationGroupbox->GetHWND(), "Z",
            glm::vec2(offset * 49.0f, offset * 5.0f), glm::vec2(offset * 3.0f, offset * 5.0f)
        );
        editorRotationZ.Textbox = new cEditorTextbox(objectRotationGroupbox->GetHWND(), "0",
            glm::vec2(offset * 54.0f, offset * 5.0f), glm::vec2(offset * 18.0f, offset * 5.0f), K_TRUE
        );
        editorScaleX.Label = new cEditorLabel(objectScaleGroupbox->GetHWND(), "X",
            glm::vec2(offset * 3.0f, offset * 5.0f), glm::vec2(offset * 3.0f, offset * 5.0f)
        );
        editorScaleX.Textbox = new cEditorTextbox(objectScaleGroupbox->GetHWND(), "1",
            glm::vec2(offset * 8.0f, offset * 5.0f), glm::vec2(offset * 18.0f, offset * 5.0f), K_TRUE
        );
        editorScaleY.Label = new cEditorLabel(objectScaleGroupbox->GetHWND(), "Y",
            glm::vec2(offset * 26.0f, offset * 5.0f), glm::vec2(offset * 3.0f, offset * 5.0f)
        );
        editorScaleY.Textbox = new cEditorTextbox(objectScaleGroupbox->GetHWND(), "1",
            glm::vec2(offset * 31.0f, offset * 5.0f), glm::vec2(offset * 18.0f, offset * 5.0f), K_TRUE
        );
        editorScaleZ.Label = new cEditorLabel(objectScaleGroupbox->GetHWND(), "Z",
            glm::vec2(offset * 49.0f, offset * 5.0f), glm::vec2(offset * 3.0f, offset * 5.0f)
        );
        editorScaleZ.Textbox = new cEditorTextbox(objectScaleGroupbox->GetHWND(), "1",
            glm::vec2(offset * 54.0f, offset * 5.0f), glm::vec2(offset * 18.0f, offset * 5.0f), K_TRUE
        );

        userInputManager->FocusWindow();
    }

    virtual void Update() override final
    {
        static float deltaTime = 0.0f;
        static float lastTime = clock();

        userInputManager->PollEvents();
        userInputManager->Update();

        // Left mouse button press
        realware::core::boolean lmbPress = K_FALSE;
        static realware::core::boolean lmbPressGlobal = K_FALSE;
        if (userInputManager->GetMouseKey(GLFW_MOUSE_BUTTON_LEFT)
            == mUserInput::eButtonState::PRESSED && lmbPressGlobal == K_FALSE) {
            lmbPress = lmbPressGlobal = K_TRUE;
        } else if (userInputManager->GetMouseKey(GLFW_MOUSE_BUTTON_LEFT)
            == mUserInput::eButtonState::RELEASED) {
            lmbPressGlobal = K_FALSE;
        }

        // Right mouse button press
        realware::core::boolean rmbPress = K_FALSE;
        static realware::core::boolean rmbPressGlobal = K_FALSE;
        if (userInputManager->GetMouseKey(GLFW_MOUSE_BUTTON_RIGHT)
            == mUserInput::eButtonState::PRESSED && rmbPressGlobal == K_FALSE) {
            rmbPress = rmbPressGlobal = K_TRUE;
        } else if (userInputManager->GetMouseKey(GLFW_MOUSE_BUTTON_RIGHT)
            == mUserInput::eButtonState::RELEASED) {
            rmbPressGlobal = K_FALSE;
        }
        
        // Create/transform entity and change selection mode
        switch (editorSelectMode)
        {
            case eSelectMode::NONE:
            {
                // Ray triangle intersection
                realware::core::entity resultEntity = 0;
                realware::core::boolean resultBool = K_FALSE;
                glm::vec3 result = glm::vec3(0.0f);
                cRayHit ray(
                    this,
                    m_scene,
                    {},
                    {},
                    m_camera,
                    userInputManager->GetCursorPosition(),
                    userInputManager->GetWindowSize(),
                    resultEntity,
                    resultBool,
                    result
                );

                if (rmbPress == K_TRUE)
                {
                    editorSelectMode = eSelectMode::CREATE;
                }
                
                break;
            }

            case eSelectMode::CREATE:
            {
                // Ray triangle intersection
                realware::core::entity resultEntity = 0;
                realware::core::boolean resultBool = K_FALSE;
                glm::vec3 result = glm::vec3(0.0f);
                cRayHit ray(
                    this,
                    m_scene,
                    {},
                    {},
                    m_camera,
                    userInputManager->GetCursorPosition(),
                    userInputManager->GetWindowSize(),
                    resultEntity,
                    resultBool,
                    result
                );

                if (rmbPress == K_TRUE)
                {
                    // Create taburet entity
                    static int i = 0;
                    auto taburet = m_scene->CreateEntity("TaburetEntity" + std::to_string(i++));
                    sCGeometryInfo* taburetgi = m_scene->Add<sCGeometryInfo>(taburet);
                    taburetgi->IsVisible = K_TRUE;
                    taburetgi->IsOpaque = K_TRUE;
                    sCGeometry* tabureg = m_scene->Add<sCGeometry>(taburet);
                    tabureg->Geometry = m_geomTaburet;
                    sCTransform* taburett = m_scene->Add<sCTransform>(taburet);
                    taburett->Position = result;
                    taburett->Rotation = glm::vec3(glm::radians(-90.0f), 0.0f, 0.0f);
                    taburett->Scale = glm::vec3(1.0f);
                    sCMaterial* planem = m_scene->Add<sCMaterial>(taburet);
                    planem->DiffuseTexture = m_taburetTexture;
                    planem->DiffuseColor = glm::vec4(1.0f);
                }

                if (lmbPress == K_TRUE && resultBool == K_TRUE)
                {
                    editorSelectMode = eSelectMode::TRANSFORM;
                    editorSelectedEntity = resultEntity;

                    sCMaterial* material = m_scene->Get<sCMaterial>(editorSelectedEntity);
                    material->DiffuseColor = glm::vec4(1.0f, 0.25f, 0.25f, 1.0f);

                    sCTransform* transform = m_scene->Get<sCTransform>(editorSelectedEntity);
                    EditorUpdateTextboxTransform(transform);
                }

                break;
            }

            case eSelectMode::TRANSFORM:
            {
                // Ray triangle intersection
                realware::core::entity resultEntity = 0;
                realware::core::boolean resultBool = K_FALSE;
                glm::vec3 result = glm::vec3(0.0f);

                if (lmbPress == K_TRUE)
                {
                    cRayHit ray(
                        this,
                        m_scene,
                        {},
                        {},
                        m_camera,
                        userInputManager->GetCursorPosition(),
                        userInputManager->GetWindowSize(),
                        resultEntity,
                        resultBool,
                        result
                    );

                    if (resultBool == K_TRUE && resultEntity != editorSelectedEntity)
                    {
                        sCMaterial* material = m_scene->Get<sCMaterial>(editorSelectedEntity);
                        material->DiffuseColor = glm::vec4(1.0f);

                        editorSelectedEntity = resultEntity;

                        material = m_scene->Get<sCMaterial>(editorSelectedEntity);
                        material->DiffuseColor = glm::vec4(1.0f, 0.25f, 0.25f, 1.0f);

                        sCTransform* transform = m_scene->Get<sCTransform>(editorSelectedEntity);
                        EditorUpdateTextboxTransform(transform);
                    }
                    else if (resultBool == K_FALSE || resultEntity == editorSelectedEntity)
                    {
                        sCMaterial* material = m_scene->Get<sCMaterial>(editorSelectedEntity);
                        material->DiffuseColor = glm::vec4(1.0f);

                        editorSelectMode = eSelectMode::CREATE;
                        editorSelectedEntity = 0;

                        editorPositionX.Textbox->SetText("0.0");
                        editorPositionY.Textbox->SetText("0.0");
                        editorPositionZ.Textbox->SetText("0.0");
                        editorRotationX.Textbox->SetText("0.0");
                        editorRotationY.Textbox->SetText("0.0");
                        editorRotationZ.Textbox->SetText("0.0");
                        editorScaleX.Textbox->SetText("0.0");
                        editorScaleY.Textbox->SetText("0.0");
                        editorScaleZ.Textbox->SetText("0.0");
                    }
                }

                if (rmbPress == K_TRUE)
                {
                    cRayHit ray(
                        this,
                        m_scene,
                        {},
                        { editorSelectedEntity },
                        m_camera,
                        userInputManager->GetCursorPosition(),
                        userInputManager->GetWindowSize(),
                        resultEntity,
                        resultBool,
                        result
                    );

                    if (resultBool == K_TRUE)
                    {
                        sCTransform* transform = m_scene->Get<sCTransform>(editorSelectedEntity);
                        transform->Position = result;

                        EditorUpdateTextboxTransform(transform);
                    }
                }

                break;
            }
        }

        // Discard entity selection
        if (userInputManager->GetKey(GLFW_KEY_X) == K_TRUE &&
            editorSelectMode == eSelectMode::TRANSFORM)
        {
            sCMaterial* material = m_scene->Get<sCMaterial>(editorSelectedEntity);
            material->DiffuseColor = glm::vec4(1.0f);

            editorSelectMode = eSelectMode::CREATE;
            editorSelectedEntity = 0;
        }

        EditorUpdateEntityTransform(m_scene);

        physicsManager->Update();

        cameraManager->Update(m_camera, m_scene, K_FALSE, K_TRUE);
        if (userInputManager->GetKey(GLFW_KEY_LEFT_ALT) == K_TRUE) {
            cameraManager->Update(m_camera, m_scene, K_TRUE, K_FALSE);
        }

        // Draw
        renderManager->SetCamera(m_camera);
        renderManager->ClearRenderPasses(glm::vec4(1.0f), 1.0f);
        renderManager->DrawGeometryOpaque(this, m_geomPlane, m_scene);
        renderManager->DrawGeometryOpaque(this, m_geomTaburet, m_scene);
        renderManager->CompositeFinal();

        userInputManager->SwapBuffers();

        clock_t t = clock();

        deltaTime = t - lastTime;
        lastTime += deltaTime;
    }

    virtual void Free() override final
    {
        fontManager->Free();
    }

private:
    cScene* m_scene;
    sVertexBufferGeometry* m_geomPlane;
    sVertexBufferGeometry* m_geomTaburet;
    entity m_camera;
    entity m_plane;
    sArea* m_taburetTexture;

};

int main()
{
    glm::vec2 monitorSize = userInputManager->GetMonitorSize();

    sApplicationDescriptor appDesc;
    appDesc.WindowDesc.Title = "Render";
    appDesc.WindowDesc.Width = monitorSize.x / 1.9f;
    appDesc.WindowDesc.Height = monitorSize.y / 1.58f;
    appDesc.WindowDesc.IsFullscreen = K_FALSE;

    MyApp app = MyApp(appDesc);
    app.Run();

    return 0;
}

void EditorUpdateTextboxTransform(sCTransform* transform)
{
    // Position
    std::string posXStr = std::to_string(transform->Position.x); posXStr.resize(5);
    editorPositionX.Textbox->SetText(posXStr);
    std::string posYStr = std::to_string(transform->Position.y); posYStr.resize(5);
    editorPositionY.Textbox->SetText(posYStr);
    std::string posZStr = std::to_string(transform->Position.z); posZStr.resize(5);
    editorPositionZ.Textbox->SetText(posZStr);
    editorPosition = glm::vec3(std::stof(posXStr), std::stof(posYStr), std::stof(posZStr));

    // Rotation
    std::string rotXStr = std::to_string(glm::degrees(transform->Rotation.x)); rotXStr.resize(5);
    editorRotationX.Textbox->SetText(rotXStr);
    std::string rotYStr = std::to_string(glm::degrees(transform->Rotation.y)); rotYStr.resize(5);
    editorRotationY.Textbox->SetText(rotYStr);
    std::string rotZStr = std::to_string(glm::degrees(transform->Rotation.z)); rotZStr.resize(5);
    editorRotationZ.Textbox->SetText(rotZStr);
    editorRotation = glm::vec3(std::stof(rotXStr), std::stof(rotYStr), std::stof(rotZStr));

    // Scale 
    std::string sclXStr = std::to_string(transform->Scale.x); sclXStr.resize(5);
    editorScaleX.Textbox->SetText(sclXStr);
    std::string sclYStr = std::to_string(transform->Scale.y); sclYStr.resize(5);
    editorScaleY.Textbox->SetText(sclYStr);
    std::string sclZStr = std::to_string(transform->Scale.z); sclZStr.resize(5);
    editorScaleZ.Textbox->SetText(sclZStr);
    editorScale = glm::vec3(std::stof(sclXStr), std::stof(sclYStr), std::stof(sclZStr));
}

void EditorUpdateEntityTransform(cScene* scene)
{
    if (editorSelectedEntity == 0) {
        return;
    }

    // Position
    std::string str1 = editorPositionX.Textbox->GetText(6);
    if (str1 == "") { str1 = "0.0"; }
    for (s32 i = 0; i < str1.size(); i++) { if (str1[1] < '.' || str1[i] > '9') { str1[i] = '0'; } }
    float textPosX = std::stof(str1);
    if (textPosX != editorPosition.x)
    {
        sCTransform* transform = scene->Get<sCTransform>(editorSelectedEntity);
        transform->Position.x = textPosX;
        editorPosition.x = textPosX;
    }
    std::string str2 = editorPositionY.Textbox->GetText(6);
    if (str2 == "") { str2 = "0.0"; }
    for (s32 i = 0; i < str2.size(); i++) { if (str2[1] < '.' || str2[i] > '9') { str2[i] = '0'; } }
    float textPosY = std::stof(str2);
    if (textPosY != editorPosition.y)
    {
        sCTransform* transform = scene->Get<sCTransform>(editorSelectedEntity);
        transform->Position.y = textPosY;
        editorPosition.y = textPosY;
    }
    std::string str3 = editorPositionZ.Textbox->GetText(6);
    if (str3 == "") { str3 = "0.0"; }
    for (s32 i = 0; i < str3.size(); i++) { if (str3[1] < '.' || str3[i] > '9') { str3[i] = '0'; } }
    float textPosZ = std::stof(str3);
    if (textPosZ != editorPosition.z)
    {
        sCTransform* transform = scene->Get<sCTransform>(editorSelectedEntity);
        transform->Position.z = textPosZ;
        editorPosition.z = textPosZ;
    }

    // Rotation
    std::string str4 = editorRotationX.Textbox->GetText(6);
    if (str4 == "") { str4 = "0.0"; }
    for (s32 i = 0; i < str4.size(); i++) { if (str4[1] < '.' || str4[i] > '9') { str4[i] = '0'; } }
    float textRotX = std::stof(str4);
    if (textRotX != editorRotation.x)
    {
        sCTransform* transform = scene->Get<sCTransform>(editorSelectedEntity);
        transform->Rotation.x = glm::radians(textRotX);
        editorRotation.x = textRotX;
    }
    std::string str5 = editorRotationY.Textbox->GetText(6);
    if (str5 == "") { str5 = "0.0"; }
    for (s32 i = 0; i < str5.size(); i++) { if (str5[1] < '.' || str5[i] > '9') { str5[i] = '0'; } }
    float textRotY = std::stof(str5);
    if (textRotY != editorRotation.y)
    {
        sCTransform* transform = scene->Get<sCTransform>(editorSelectedEntity);
        transform->Rotation.y = glm::radians(textRotY);
        editorRotation.y = textRotY;
    }
    std::string str6 = editorRotationZ.Textbox->GetText(6);
    if (str6 == "") { str6 = "0.0"; }
    for (s32 i = 0; i < str6.size(); i++) { if (str6[1] < '.' || str6[i] > '9') { str6[i] = '0'; } }
    float textRotZ = std::stof(str6);
    if (textRotZ != editorRotation.z)
    {
        sCTransform* transform = scene->Get<sCTransform>(editorSelectedEntity);
        transform->Rotation.z = glm::radians(textRotZ);
        editorRotation.z = textRotZ;
    }

    // Scale
    std::string str7 = editorScaleX.Textbox->GetText(6);
    if (str7 == "") { str7 = "0.0"; }
    for (s32 i = 0; i < str7.size(); i++) { if (str7[1] < '.' || str7[i] > '9') { str7[i] = '0'; } }
    float textSclX = std::stof(str7);
    if (textSclX != editorScale.x)
    {
        sCTransform* transform = scene->Get<sCTransform>(editorSelectedEntity);
        transform->Scale.x = textSclX;
        editorScale.x = textSclX;
    }
    std::string str8 = editorScaleY.Textbox->GetText(6);
    if (str8 == "") { str8 = "0.0"; }
    for (s32 i = 0; i < str8.size(); i++) { if (str8[1] < '.' || str8[i] > '9') { str8[i] = '0'; } }
    float textSclY = std::stof(str8);
    if (textSclY != editorScale.y)
    {
        sCTransform* transform = scene->Get<sCTransform>(editorSelectedEntity);
        transform->Scale.y = textSclY;
        editorScale.y = textSclY;
    }
    std::string str9 = editorScaleZ.Textbox->GetText(6);
    if (str9 == "") { str3 = "0.0"; }
    for (s32 i = 0; i < str9.size(); i++) { if (str9[1] < '.' || str9[i] > '9') { str9[i] = '0'; } }
    float textSclZ = std::stof(str9);
    if (textSclZ != editorScale.z)
    {
        sCTransform* transform = scene->Get<sCTransform>(editorSelectedEntity);
        transform->Scale.z = textSclZ;
        editorScale.z = textSclZ;
    }
}

void EditorWindowAssetShowPopupmenu(realware::core::boolean rmbPress)
{
    if (rmbPress == realware::core::K_FALSE) {
        return;
    }

    HMENU hPopupMenu = CreatePopupMenu();

    POINT p = {};
    GetCursorPos(&p);

    AppendMenu(hPopupMenu, MF_STRING, 1, "New");
    AppendMenu(hPopupMenu, MF_STRING, 2, "Delete");

    TrackPopupMenu(
        hPopupMenu,
        TPM_LEFTALIGN | TPM_RIGHTBUTTON,
        p.x,
        p.y,
        0,
        editorWindowAssetListView->GetHWND(),
        nullptr
    );

    DestroyMenu(hPopupMenu);
}

void EditorWindowEntityUpdate(int assetIndex)
{
    sAsset& asset = editorWindowAssetData[(int)editorWindowAssetSelectedType][assetIndex];
    std::string caption = "Entity - " + asset.Name;
    SetWindowText(editorWindowEntity->GetHWND(), caption.data());
    editorWindowEntityName.Textbox->SetText(asset.Name);
}

void EditorWindowEntitySave(int assetIndex)
{
    sAsset& asset = editorWindowAssetData[(int)editorWindowAssetSelectedType][assetIndex];
    asset.Name = editorWindowEntityName.Textbox->GetText(100);

    std::string caption = "Entity - " + asset.Name;
    SetWindowText(editorWindowEntity->GetHWND(), caption.data());
    ListView_SetItemText(editorWindowAssetListView->GetHWND(), assetIndex, 0, asset.Name.data());
}