#include <iostream>
#include <chrono>
#include <ctime>
#include <vector>

#include "../../engine/thirdparty/glm/glm/glm.hpp"
#include "../../engine/thirdparty/glm/glm/gtc/matrix_transform.hpp"
#include "../../engine/src/core/application.hpp"
#include "../../engine/src/core/ecs.hpp"
#include "../../engine/src/core/user_input_manager.hpp"
#include "../../engine/src/core/camera_manager.hpp"
#include "../../engine/src/core/texture_manager.hpp"
#include "../../engine/src/render/render_manager.hpp"
#include "../../engine/src/render/render_context.hpp"
#include "../../engine/src/sound/sound_context.hpp"
#include "../../engine/src/sound/sound_manager.hpp"
#include "../../engine/src/font/font_manager.hpp"
#include "../../engine/src/ui/widget_manager.hpp"

using namespace realware::core;
using namespace realware::render;
using namespace realware::font;
using namespace realware::sound;

class MyApp : public cApplication
{

public:
    MyApp(const sApplicationDescriptor& desc) : cApplication(desc) {}
    ~MyApp() {}

    virtual void Init() override final
    {
        // Initialize managers
        textureManager->Init(m_renderContext, 1920, 1080, 12);
        mRender::Init(m_renderContext, 65536, 65536, 65536, 65536, glm::vec2(m_desc.WindowDesc.Width, m_desc.WindowDesc.Height));
        mFont::Init(m_renderContext);
        mSound::Init(m_soundContext);

        // Triangle primitive
        auto triangle = mRender::CreateTriangle();
        m_geometry = mRender::AddGeometry(
            triangle->Format,
            triangle->VerticesByteSize,
            triangle->Vertices,
            triangle->IndicesByteSize,
            triangle->Indices
        );

        // Scene
        m_scene = new cScene(65536);

        // Triangle entity
        entity triangleEntity = m_scene->CreateEntity("TriangleEntity");
        sCGeometryInfo* triangleGeometryInfo = m_scene->Add<sCGeometryInfo>(triangleEntity);
        triangleGeometryInfo->IsVisible = K_TRUE;
        triangleGeometryInfo->IsOpaque = K_TRUE;
        sCGeometry* triangleGeometry = m_scene->Add<sCGeometry>(triangleEntity);
        triangleGeometry->Geometry = m_geometry;
        sCTransform* triangleTransform = m_scene->Add<sCTransform>(triangleEntity);
        triangleTransform->Position = glm::vec3(0.0f, 0.0f, -1.0f);
        triangleTransform->Scale = glm::vec3(1.0f);
        sCMaterial* triangleMaterial = m_scene->Add<sCMaterial>(triangleEntity);
        triangleMaterial->DiffuseTexture = nullptr;
        triangleMaterial->DiffuseColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);

        // Triangle transparent entity
        entity triangleEntityTransparent = m_scene->CreateEntity("TriangleTransparentEntity");
        sCGeometryInfo* triangleTransparentGeometryInfo = m_scene->Add<sCGeometryInfo>(triangleEntityTransparent);
        triangleTransparentGeometryInfo->IsVisible = K_TRUE;
        triangleTransparentGeometryInfo->IsOpaque = K_FALSE;
        sCGeometry* triangleTransparentGeometry = m_scene->Add<sCGeometry>(triangleEntityTransparent);
        triangleTransparentGeometry->Geometry = m_geometry;
        sCTransform* triangleTransparentTransform = m_scene->Add<sCTransform>(triangleEntityTransparent);
        triangleTransparentTransform->Position = glm::vec3(0.0f, 0.0f, 0.0f);
        triangleTransparentTransform->Scale = glm::vec3(1.0f);
        sCMaterial* triangleTransparentMaterial = m_scene->Add<sCMaterial>(triangleEntityTransparent);
        triangleTransparentMaterial->DiffuseTexture = nullptr;
        triangleTransparentMaterial->DiffuseColor = glm::vec4(0.0f, 0.0f, 1.0f, 0.25f);

        // Camera entity
        m_camera = m_scene->CreateEntity("CameraEntity");
        sCCamera* cameraCamera = m_scene->Add<sCCamera>(m_camera);
        cameraCamera->FOV = 65.0f;
        cameraCamera->ZNear = 0.01f;
        cameraCamera->ZFar = 100.0f;
        sCTransform* cameraTransform = m_scene->Add<sCTransform>(m_camera);
        cameraTransform->Position = glm::vec3(0.0f);
    }

    virtual void Update() override final
    {
        static float deltaTime = 0.0f;
        static float lastTime = clock();

        mUserInput::PollEvents();
        mUserInput::Update();

        mCamera::Update(m_camera, m_scene);

        // Draw
        mRender::DrawGeometryOpaque(this, m_geometry, m_camera, m_scene, glm::vec4(1.0f), 1.0f);
        mRender::DrawGeometryTransparent(this, m_geometry, m_camera, m_scene);
        mRender::CompositeTransparent();
        mRender::CompositeFinal();

        mUserInput::SwapBuffers();

        clock_t t = clock();

        deltaTime = t - lastTime;
        lastTime += deltaTime;
    }

    virtual void Free() override final
    {
        mFont::Free();
    }

private:
    cScene* m_scene;
    sVertexBufferGeometry* m_geometryPlane;
    entity m_camera;

};

int main()
{
    sApplicationDescriptor appDesc;
    appDesc.WindowDesc.Title = "Test Window";
    appDesc.WindowDesc.Width = 1366;
    appDesc.WindowDesc.Height = 768;
    appDesc.WindowDesc.IsFullscreen = K_TRUE;

    MyApp app = MyApp(appDesc);
    app.Run();

    return 0;
}