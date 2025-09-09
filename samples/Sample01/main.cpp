#include <iostream>
#include <chrono>
#include <ctime>
#include <vector>

#include <GL/glew.h>
#include "../../engine/thirdparty/glm/glm/glm.hpp"
#include "../../engine/thirdparty/glm/glm/gtc/matrix_transform.hpp"
#include "../../engine/src/application.hpp"
#include "../../engine/src/ecs.hpp"
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

using namespace realware::core;
using namespace realware::render;
using namespace realware::font;
using namespace realware::sound;
using namespace realware::physics;

class MyApp : public cApplication
{
public:
    MyApp(const sApplicationDescriptor* const desc) : cApplication(desc) {}
    ~MyApp()
    {
        _render->DestroyPrimitive(_trianglePrimitive);
        delete _triangleGeometry;
    }

    virtual void Pre() override final
    {
        // Create camera
        _camera->CreateCamera();

        // Triangle geometry
        _trianglePrimitive = _render->CreatePrimitive(mRender::ePrimitive::TRIANGLE);
        _triangleGeometry = _render->AddGeometry(
            _trianglePrimitive->Format,
            _trianglePrimitive->VerticesByteSize,
            _trianglePrimitive->Vertices,
            _trianglePrimitive->IndicesByteSize,
            _trianglePrimitive->Indices
        );

        // Materials
        cMaterial* material1 = _render->CreateMaterial(
            "Material1",
            nullptr,
            glm::vec4(0.0f, 1.0f, 0.0f, 1.0f),
            glm::vec4(1.0f)
        );
        cMaterial* material2 = _render->CreateMaterial(
            "Material2",
            nullptr,
            glm::vec4(1.0f, 0.0f, 0.0f, 0.5f),
            glm::vec4(1.0f)
        );
        
        // Font
        sFont* font = _font->NewFont(
            "C:/DDD/RealWare/resources/fonts/brahms.ttf",
            64
        );

        // Text
        cText* text = new cText(font, "New line\ntest\nanother newline\there");

        // Game objects
        cGameObject* triangleObject1 = _gameObject->CreateGameObject("TriangleObject1");
        triangleObject1->SetVisible(K_TRUE);
        triangleObject1->SetOpaque(K_TRUE);
        triangleObject1->SetGeometry(_triangleGeometry);
        triangleObject1->SetPosition(glm::vec3(0.0f, 0.0f, -1.0f));
        triangleObject1->SetScale(glm::vec3(1.0f));
        triangleObject1->SetMaterial(material1);
        cGameObject* triangleObject2 = _gameObject->CreateGameObject("TriangleObject2");
        triangleObject2->SetVisible(K_TRUE);
        triangleObject2->SetOpaque(K_FALSE);
        triangleObject2->SetGeometry(_triangleGeometry);
        triangleObject2->SetPosition(glm::vec3(0.0f, 0.0f, -3.0f));
        triangleObject2->SetScale(glm::vec3(1.0f));
        triangleObject2->SetMaterial(material2);
        cGameObject* textObject = _gameObject->CreateGameObject("TextObject");
        textObject->SetVisible(K_TRUE);
        textObject->SetOpaque(K_TRUE);
        textObject->SetPosition(glm::vec3(0.5f, 0.5f, 0.0f));
        textObject->SetScale(glm::vec3(1.0f));
        textObject->SetMaterial(material1);
        textObject->SetText(text);
    }

    virtual void Update() override final
    {
        static float deltaTime = 0.0f;
        static float lastTime = clock();

        _renderContext->ClearColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        _renderContext->ClearDepth(1.0f);

        _camera->Update(K_TRUE, K_TRUE);

        // Draw
        cGameObject* cameraObject = _camera->GetCamera();
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
        //this->GetRenderManager()->DrawTexts(
        //    this,
        //    this->GetGameObjectManager()->GetObjects()
        //);
        _render->CompositeFinal();

        clock_t t = clock();

        deltaTime = t - lastTime;
        lastTime += deltaTime;
    }

    virtual void Post() override final
    {
    }

private:
    cScene* m_scene = nullptr;
    sPrimitive* _trianglePrimitive = nullptr;
    sVertexBufferGeometry* _triangleGeometry = nullptr;
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