#include <iostream>
#include <chrono>
#include <ctime>
#include <vector>

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
#include "../../engine/src/widget_manager.hpp"
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
    MyApp(const sApplicationDescriptor& desc) : cApplication(desc) {}
    ~MyApp() {}

    virtual void Init() override final
    {
        // Triangle primitive
        auto triangle = this->GetRenderManager()->CreateTriangle();
        m_geometryTriangle = this->GetRenderManager()->AddGeometry(
            triangle->Format,
            triangle->VerticesByteSize,
            triangle->Vertices,
            triangle->IndicesByteSize,
            triangle->Indices
        );

        cMaterial* material1 = this->GetRenderManager()->CreateMaterial(
            "Material1",
            nullptr,
            glm::vec4(0.0f, 1.0f, 0.0f, 1.0f),
            glm::vec4(1.0f)
        );

        cMaterial* material2 = this->GetRenderManager()->CreateMaterial(
            "Material2",
            nullptr,
            glm::vec4(1.0f, 0.0f, 0.0f, 0.5f),
            glm::vec4(1.0f)
        );

        cGameObject* triangleObject1 = this->GetGameObjectManager()->CreateGameObject("TriangleObject1");
        triangleObject1->SetVisible(K_TRUE);
        triangleObject1->SetOpaque(K_TRUE);
        triangleObject1->SetGeometry(m_geometryTriangle);
        triangleObject1->SetPosition(glm::vec3(0.0f, 0.0f, -1.0f));
        triangleObject1->SetScale(glm::vec3(1.0f));
        triangleObject1->SetMaterial(material1);

        cGameObject* triangleObject2 = this->GetGameObjectManager()->CreateGameObject("TriangleObject2");
        triangleObject2->SetVisible(K_TRUE);
        triangleObject2->SetOpaque(K_FALSE);
        triangleObject2->SetGeometry(m_geometryTriangle);
        triangleObject2->SetPosition(glm::vec3(0.0f, 0.0f, -3.0f));
        triangleObject2->SetScale(glm::vec3(1.0f));
        triangleObject2->SetMaterial(material2);

        cGameObject* cameraObject = this->GetGameObjectManager()->CreateGameObject("CameraObject");
        
        sFont* font = this->GetFontManager()->NewFont(
            "C:/DDD/RealWare/resources/fonts/brahms.ttf",
            64,
            1,
            1,
            1
        );
        cText* text = new cText(font, "New line\ntest\nanother newline");

        cGameObject* textObject = this->GetGameObjectManager()->CreateGameObject("TextObject");
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

        this->PollEvents();

        this->GetCameraManager()->Update(K_TRUE, K_TRUE);
        this->GetGameObjectManager()->FindGameObject("CameraObject")->SetViewProjectionMatrix(
            this->GetCameraManager()->GetViewProjectionMatrix()
        );

        // Draw
        this->GetRenderManager()->ClearRenderPasses(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), 1.0f);
        this->GetRenderManager()->DrawGeometryOpaque(
            this,
            m_geometryTriangle,
            this->GetGameObjectManager()->GetObjects(),
            "CameraObject"
        );
        this->GetRenderManager()->DrawGeometryTransparent(
            this,
            m_geometryTriangle,
            this->GetGameObjectManager()->GetObjects(),
            "CameraObject"
        );
        this->GetRenderManager()->DrawTexts(
            this,
            this->GetGameObjectManager()->GetObjects()
        );
        this->GetRenderManager()->CompositeTransparent();
        this->GetRenderManager()->CompositeFinal();

        this->SwapBuffers();

        clock_t t = clock();

        deltaTime = t - lastTime;
        lastTime += deltaTime;
    }

    virtual void Free() override final
    {
        this->FreeManagers();
    }

private:
    cScene* m_scene;
    sVertexBufferGeometry* m_geometryTriangle;
    entity m_camera;

};

int main()
{
    sApplicationDescriptor appDesc;
    appDesc.WindowDesc.Title = "Test Window";
    appDesc.WindowDesc.Width = 640;
    appDesc.WindowDesc.Height = 480;
    appDesc.WindowDesc.IsFullscreen = K_FALSE;

    MyApp app = MyApp(appDesc);
    app.Run();

    return 0;
}