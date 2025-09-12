#include <iostream>
#include <chrono>
#include <ctime>
#include <vector>

#include <GL/glew.h>
#include "../../engine/thirdparty/glm/glm/glm.hpp"
#include "../../engine/thirdparty/glm/glm/gtc/matrix_transform.hpp"
#include "../../engine/src/application.hpp"
#include "../../engine/src/camera_manager.hpp"
#include "../../engine/src/texture_manager.hpp"
#include "../../engine/src/render_manager.hpp"
#include "../../engine/src/render_context.hpp"
#include "../../engine/src/sound_context.hpp"
#include "../../engine/src/sound_manager.hpp"
#include "../../engine/src/font_manager.hpp"
#include "../../engine/src/filesystem_manager.hpp"
#include "../../engine/src/physics_manager.hpp"
#include "../../engine/src/gameobject_manager.hpp"

using namespace realware::app;
using namespace realware::game;
using namespace realware::render;
using namespace realware::font;
using namespace realware::sound;
using namespace realware::physics;
using namespace types;

class MyApp : public cApplication
{
public:
    MyApp(const sApplicationDescriptor* const desc) : cApplication(desc) {}
    ~MyApp()
    {
        _render->DestroyGeometry(_triangleGeometry);
        _render->DestroyPrimitive(_trianglePrimitive);
    }

    virtual void Pre() override final
    {
        // Triangle geometry
        _trianglePrimitive = _render->CreatePrimitive(mRender::ePrimitive::TRIANGLE);
        _triangleGeometry = _render->CreateGeometry(
            _trianglePrimitive->Format,
            _trianglePrimitive->VerticesByteSize,
            _trianglePrimitive->Vertices,
            _trianglePrimitive->IndicesByteSize,
            _trianglePrimitive->Indices
        );

        // Textures
        sTextureAtlasTexture* texture1 = _texture->AddTexture(
            "Texture1",
            "C:/DDD/RealWare/resources/texture1.png"
        );
        sTextureAtlasTexture* texture2 = _texture->AddTexture(
            "Texture2",
            "C:/DDD/RealWare/resources/texture2.png"
        );

        // Materials
        cMaterial* material1 = _render->AddMaterial(
            "Material1",
            texture1,
            glm::vec4(0.0f, 1.0f, 0.0f, 1.0f),
            glm::vec4(1.0f)
        );
        cMaterial* material2 = _render->AddMaterial(
            "Material2",
            texture2,
            glm::vec4(1.0f, 0.0f, 0.0f, 0.5f),
            glm::vec4(1.0f)
        );
        
        // Font
        sFont* font = _font->CreateFontTTF(
            "C:/DDD/RealWare/resources/fonts/brahms.ttf",
            64
        );

        // Text
        cText* text = new cText(font, "New line\ntest\nanother newline\there");

        // Physics
        cSimulationScene* pxScene = _physics->AddScene("PXScene1");
        cSubstance* pxSubstance = _physics->AddSubstance("PXSubstance1");

        // Game objects
        cGameObject* triangleObject1 = _gameObject->AddGameObject("TriangleObject1");
        triangleObject1->SetVisible(K_TRUE);
        triangleObject1->SetOpaque(K_TRUE);
        triangleObject1->SetGeometry(_triangleGeometry);
        triangleObject1->SetPosition(glm::vec3(0.0f, 0.0f, -1.0f));
        triangleObject1->SetScale(glm::vec3(1.0f));
        triangleObject1->SetMaterial(material1);
        triangleObject1->SetPhysicsActor(
            GameObjectFeatures::PHYSICS_ACTOR_STATIC,
            GameObjectFeatures::PHYSICS_SHAPE_PLANE,
            pxScene,
            pxSubstance,
            0.0f
        );

        cGameObject* triangleObject2 = _gameObject->AddGameObject("TriangleObject2");
        triangleObject2->SetVisible(K_TRUE);
        triangleObject2->SetOpaque(K_FALSE);
        triangleObject2->SetGeometry(_triangleGeometry);
        triangleObject2->SetPosition(glm::vec3(0.0f, 0.0f, -3.0f));
        triangleObject2->SetScale(glm::vec3(1.0f));
        triangleObject2->SetMaterial(material2);

        cGameObject* textObject = _gameObject->AddGameObject("TextObject");
        textObject->SetVisible(K_TRUE);
        textObject->SetOpaque(K_TRUE);
        textObject->SetPosition(glm::vec3(0.5f, 0.5f, 0.0f));
        textObject->SetScale(glm::vec3(1.0f));
        textObject->SetMaterial(material1);
        textObject->SetText(text);

        // Create camera
        _camera->CreateCamera();
        _camera->SetMoveSpeed(5.0f);
        _cameraGameObject = _camera->GetCameraGameObject();
        _cameraGameObject->SetPosition(glm::vec3(0.0f, 5.0f, 0.0f));
        _cameraGameObject->SetPhysicsController(
            0.0f,
            0.51f,
            0.5f,
            glm::vec3(0.0f, 1.0f, 0.0f),
            pxScene,
            pxSubstance
        );
    }

    virtual void Update() override final
    {
        // Physics
        _physics->Simulate();

        // Camera
        _camera->Update(K_TRUE, K_TRUE);

        // Rendering
        _renderContext->ClearColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        _renderContext->ClearDepth(1.0f);

        cGameObject* cameraObject = _cameraGameObject;
        _render->ClearRenderPasses(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), 1.0f);
        _render->DrawGeometryOpaque(
            _triangleGeometry,
            _gameObject->GetObjects(),
            cameraObject
        );
        _render->DrawGeometryTransparent(
            _triangleGeometry,
            _gameObject->GetObjects(),
            cameraObject
        );
        _render->CompositeTransparent();
        _render->DrawTexts(_gameObject->GetObjects());
        _render->CompositeFinal();
    }

    virtual void Post() override final
    {
    }

private:
    sPrimitive* _trianglePrimitive = nullptr;
    sVertexBufferGeometry* _triangleGeometry = nullptr;
    cGameObject* _cameraGameObject = nullptr;
};

int main()
{
    sApplicationDescriptor* appDesc = new sApplicationDescriptor;
    appDesc->WindowDesc.Title = "Test Window";
    appDesc->WindowDesc.Width = 640;
    appDesc->WindowDesc.Height = 480;
    appDesc->WindowDesc.IsFullscreen = K_FALSE;

    MyApp* app = new MyApp(appDesc);
    app->Run();
    
    delete app;
    delete appDesc;

    /*glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    auto m_window = (void*)glfwCreateWindow(640, 480, "", nullptr, nullptr);
    glfwMakeContextCurrent((GLFWwindow*)m_window);
    glfwSwapInterval(1);
    if (glewInit() != GLEW_OK)
    {
        std::cout << "Error initializing GLEW!" << std::endl;
        return 0;
    }

    while (glfwWindowShouldClose((GLFWwindow*)m_window) == K_FALSE)
    {
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

        glfwPollEvents();
    }*/

    return 0;
}