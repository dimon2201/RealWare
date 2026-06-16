#include <iostream>
#include "capabilities.hpp"
#include "context.hpp"
#include "application.hpp"
#include "ecs_subsystem.hpp"
#include "components.hpp"
#include "graphics.hpp"

using namespace triton;
using namespace triton::ecs;
using namespace triton::ecs::components;
using namespace types;

class cMyApplication final : public IApplication
{
public:
    cMyApplication(cContext* context, const sCapabilities* caps) : IApplication(context, caps)
    {
    }

    virtual ~cMyApplication() override final
    {
    }

    virtual void Setup() override final
    {
        XECSSubsystem* ecs = _context->GetSubsystem<XECSSubsystem>();
        SSceneHandle scene1Handle = ecs->CreateScene("Scene1");
        cScene* scene1 = ecs->GetScene(scene1Handle);
        entity ent = scene1->CreateEntity();
        STransformComponent* entTransform = scene1->CreateTransformComponent(ent);
        SRenderInstanceComponent* entRender = scene1->CreateRenderInstanceComponent(ent);
        entTransform->_world = cMatrix4(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f)));
        
        SRenderData renderDataStatic = scene1->BuildRenderDataStatic();
        XRenderPass* opaqueRenderPass = _context->GetSubsystem<cGraphics>()->GetOpaqueRenderPass();
        opaqueRenderPass->UploadInstancesStatic(renderDataStatic);
    }

    virtual void Stop() override final
    {
    }
};

int main()
{
    /*
        cPlatform* platform = new cPlatform(
            cPlatform::eGraphicsBackend::OGL,
            cPlatform::eInputBackend::GLFW
        ); // тут хранятся все бэкенды
        cInputWindow* window = platform->CreatePlatformWindow(); // тут обязательны вызовы бэкэндов
        cMyApplication* myApp = new cMyApplication(context, &caps); // создается пользовательское приложение
    */

    cContext* context = new cContext();
    std::cout << "Context initialized." << std::endl;

    sCapabilities caps = {};
    caps.windowCount = 1;
    caps.windows[0].windowTitle = "My Test Application";
    caps.windows[0].windowWidth = 800;
    caps.windows[0].windowHeight = 600;
    caps.windows[0].fullscreen = K_FALSE;

    cMyApplication* myApp = new cMyApplication(context, &caps);
    std::cout << "Application initialized." << std::endl;

    myApp->Run();

    return 0;
}

/*#include <iostream>
#include <chrono>
#include <ctime>
#include <vector>

#include "../../engine/thirdparty/glm/glm/glm.hpp"
#include "../../engine/thirdparty/glm/glm/gtc/matrix_transform.hpp"
#include "../../engine/src/application.hpp"
#include "../../engine/src/context.hpp"
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
#include "../../engine/src/event_manager.hpp"
#include "../../engine/src/thread_manager.hpp"
#include "../../engine/src/memory_pool.hpp"
#include "../../engine/src/buffer.hpp"

using namespace realware;
using namespace types;

void func(cBuffer* const data)
{
}

class MyApp : public iApplication
{
public:
    MyApp(cContext* context) : iApplication(context) {}
    ~MyApp()
    {
        _render->DestroyGeometry(_triangleGeometry);
        _render->DestroyPrimitive(_trianglePrimitive);
    }

    sRenderPass* CreateCustomRenderPass()
    {
        mRender* render = GetRenderManager();
        mTexture* texture = GetTextureManager();
        iRenderContext* renderContext = GetRenderContext();
        sRenderPass* opaqueRenderPass = render->GetOpaqueRenderPass();
        sRenderPass* transparentRenderPass = render->GetTransparentRenderPass();
        const glm::vec2 windowSize = GetWindowSize();

        u8 customRenderPassTexture1Data[4 * 2 * 2] = {
            255,
            0,
            0,
            255,

            255,
            0,
            0,
            255,

            255,
            0,
            0,
            255,

            255,
            0,
            0,
            255
        };
        cTextureAtlasTexture* customRenderPassTexture1 = texture->CreateTexture(
            "CustomRenderPassTexture1",
            glm::vec2(2, 2),
            4,
            &customRenderPassTexture1Data[0]
        );
        u8 customRenderPassTexture2Data[4 * 2 * 2] = {
            255,
            255,
            255,
            255,

            255,
            255,
            255,
            255,

            255,
            255,
            255,
            255,

            255,
            255,
            255,
            255
        };
        cTextureAtlasTexture* customRenderPassTexture2 = texture->CreateTexture(
            "CustomRenderPassTexture2",
            glm::vec2(2, 2),
            4,
            &customRenderPassTexture2Data[0]
        );

        std::string vertexFunc = "";
        std::string fragmentFunc = "";
        render->LoadShaderFiles(
            "C:/DDD/RealWare/out/build/x64-Debug/samples/Sample01/data/shaders/custom_vertex.shader",
            "C:/DDD/RealWare/out/build/x64-Debug/samples/Sample01/data/shaders/custom_fragment.shader",
            vertexFunc,
            fragmentFunc
        );

        sRenderPass::sDescriptor renderPassDesc;
        renderPassDesc._inputVertexFormat = eCategory::VERTEX_BUFFER_FORMAT_POS_TEX_NRM_VEC3_VEC2_VEC3;
        renderPassDesc._inputBuffers.emplace_back(render->GetVertexBuffer());
        renderPassDesc._inputBuffers.emplace_back(render->GetIndexBuffer());
        renderPassDesc._inputBuffers.emplace_back(render->GetOpaqueInstanceBuffer());
        renderPassDesc._inputBuffers.emplace_back(render->GetOpaqueMaterialBuffer());
        renderPassDesc._inputBuffers.emplace_back(render->GetLightBuffer());
        renderPassDesc._inputBuffers.emplace_back(render->GetOpaqueTextureAtlasTexturesBuffer());
        renderPassDesc._inputTextures.emplace_back(GetTextureManager()->GetAtlas());
        renderPassDesc._inputTextureNames.emplace_back("TextureAtlas");
        renderPassDesc._inputTextureAtlasTextures.emplace_back(customRenderPassTexture1);
        renderPassDesc._inputTextureAtlasTextures.emplace_back(customRenderPassTexture2);
        renderPassDesc._inputTextureAtlasTextureNames.emplace_back("MyRedTexture");
        renderPassDesc._inputTextureAtlasTextureNames.emplace_back("MyWhiteTexture");
        renderPassDesc._shaderBase = nullptr;
        renderPassDesc._shaderRenderPath = eCategory::RENDER_PATH_OPAQUE;
        renderPassDesc._shaderVertexPath = "C:/DDD/RealWare/out/build/x64-Debug/samples/Sample01/data/shaders/main_vertex.shader";
        renderPassDesc._shaderFragmentPath = "C:/DDD/RealWare/out/build/x64-Debug/samples/Sample01/data/shaders/main_fragment.shader";
        renderPassDesc._renderTarget = opaqueRenderPass->_desc._renderTarget;
        renderPassDesc._viewport = glm::vec4(0.0f, 0.0f, windowSize.x, windowSize.y);
        renderPassDesc._depthMode._useDepthTest = K_TRUE;
        renderPassDesc._depthMode._useDepthWrite = K_TRUE;
        renderPassDesc._blendMode._factorCount = 1;
        renderPassDesc._blendMode._srcFactors[0] = sBlendMode::eFactor::ONE;
        renderPassDesc._blendMode._dstFactors[0] = sBlendMode::eFactor::ZERO;
        
        return renderContext->CreateRenderPass(renderPassDesc);
    }

    virtual void Setup() override final
    {
        // Create custom render pass
        _customRenderPass = CreateCustomRenderPass();

        // Triangle geometry
        _trianglePrimitive = _render->CreatePrimitive(eCategory::PRIMITIVE_TRIANGLE);
        _triangleGeometry = _render->CreateGeometry(
            _trianglePrimitive->_format,
            _trianglePrimitive->_verticesByteSize,
            _trianglePrimitive->_vertices,
            _trianglePrimitive->_indicesByteSize,
            _trianglePrimitive->_indices
        );
        _quadPrimitive = _render->CreatePrimitive(eCategory::PRIMITIVE_QUAD);
        _quadGeometry = _render->CreateGeometry(
            _quadPrimitive->_format,
            _quadPrimitive->_verticesByteSize,
            _quadPrimitive->_vertices,
            _quadPrimitive->_indicesByteSize,
            _quadPrimitive->_indices
        );

        // Models
        _cubeModel = _render->CreateModel("C:/DDD/RealWare/resources/cube.fbx");
        _cubeGeometry = _render->CreateGeometry(
            _cubeModel->_format,
            _cubeModel->_verticesByteSize,
            _cubeModel->_vertices,
            _cubeModel->_indicesByteSize,
            _cubeModel->_indices
        );

        // Textures
        cTextureAtlasTexture* texture1 = _texture->CreateTexture(
            "Texture1",
            "C:/DDD/RealWare/resources/texture1.png"
        );
        cTextureAtlasTexture* texture2 = _texture->CreateTexture(
            "Texture2",
            "C:/DDD/RealWare/resources/texture2.png"
        );

        // Materials
        cMaterial* material1 = _render->CreateMaterial(
            "Material1",
            texture1,
            glm::vec4(0.0f, 1.0f, 0.0f, 1.0f),
            glm::vec4(1.0f)
        );
        cMaterial* material2 = _render->CreateMaterial(
            "Material2",
            texture2,
            glm::vec4(1.0f, 0.0f, 0.0f, 0.5f),
            glm::vec4(1.0f)
        );
        
        // Text
        sFont* font = _font->CreateFontTTF("C:/DDD/RealWare/resources/brahms.ttf", 64);
        sText* text = _font->CreateText(font, "New line\ntest\nanother newline\there");

        // Physics
        cPhysicsSimulationScene* pxScene = _physics->CreateScene("PXScene1");
        cPhysicsSubstance* pxSubstance = _physics->CreateSubstance("PXSubstance1");

        // Game objects
        const usize N = 1;
        for (usize z = 0; z < N; z++)
        {
            for (usize y = 0; y < N; y++)
            {
                for (usize x = 0; x < N; x++)
                {
                    const glm::vec3 position = glm::vec3(x, y, z);
                    const std::string id = "Cube" + std::to_string(x) + std::to_string(y) + std::to_string(z);

                    cGameObject* cubeObject1 = _gameObject->CreateGameObject(id);
                    cubeObject1->SetVisible(K_TRUE);
                    cubeObject1->SetOpaque(K_TRUE);
                    cubeObject1->SetGeometry(_cubeGeometry);
                    cubeObject1->GetTransform()->_position = position;
                    cubeObject1->GetTransform()->_scale = glm::vec3(1.0f);
                    cubeObject1->SetMaterial(material2);
                }
            }
        }

        cGameObject* textObject = _gameObject->CreateGameObject("TextObject");
        textObject->SetVisible(K_TRUE);
        textObject->SetOpaque(K_TRUE);
        textObject->GetTransform()->_position = glm::vec3(0.5f, 0.5f, 0.0f);
        textObject->GetTransform()->_scale = glm::vec3(1.0f);
        textObject->SetMaterial(material1);
        textObject->SetText(text);
        
        //TaskFunction f = TaskFunction([](cBuffer* const data) {std::cout << "Success!" << std::endl;});
        //cTask task(nullptr, std::move(f));
        //GetThreadManager()->Submit(
        //    task
        //);

        EventFunction ef = EventFunction([](cBuffer* const data) {std::cout << "Success KEY_PRESS!" << std::endl;});
        GetEventManager()->Subscribe("Event1", eEvent::KEY_PRESS, textObject, std::move(ef));
        GetEventManager()->Send(eEvent::KEY_PRESS);

        // Create camera
        _camera->CreateCamera();
        _camera->SetMoveSpeed(5.0f);
        _cameraGameObject = _camera->GetCameraGameObject();
        _cameraGameObject->GetTransform()->_position = glm::vec3(0.0f, 5.0f, 0.0f);
        _cameraGameObject->SetPhysicsController(
            0.0f,
            0.51f,
            0.5f,
            glm::vec3(0.0f, 1.0f, 0.0f),
            pxScene,
            pxSubstance
        );

        _render->WriteObjectsToOpaqueBuffers(_gameObject->GetObjects(), _customRenderPass);
    }

    virtual void Update() override final
    {
        // Custom render pass
        _customRenderPass->_desc._viewport = glm::vec4(0.0f, 0.0f, GetWindowSize().x, GetWindowSize().y);

        // Rendering
        cGameObject* cameraObject = _cameraGameObject;
        _render->ClearRenderPasses(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), 1.0f);
        _render->DrawGeometryOpaque(
            _cubeGeometry,
            _cameraGameObject,
            _customRenderPass
        );
    }

    virtual void Stop() override final
    {
    }

private:
    sPrimitive* _trianglePrimitive = nullptr;
    sPrimitive* _quadPrimitive = nullptr;
    sModel* _cubeModel = nullptr;
    sVertexBufferGeometry* _triangleGeometry = nullptr;
    sVertexBufferGeometry* _quadGeometry = nullptr;
    sVertexBufferGeometry* _cubeGeometry = nullptr;
    cGameObject* _cameraGameObject = nullptr;
    sRenderPass* _customRenderPass = nullptr;
    cVector<cGameObject>* _transparentGameObjects = {};
    cVector<cGameObject>* _textGameObjects = {};
};

int main()
{
    sApplicationDescriptor* appDesc = new sApplicationDescriptor;
    appDesc->_windowDesc._title = "Test Window";
    appDesc->_windowDesc._width = 640;
    appDesc->_windowDesc._height = 480;
    appDesc->_windowDesc._isFullscreen = K_FALSE;
    appDesc->_memoryPoolByteSize = 128 * 1024 * 1024;
    appDesc->_maxGameObjectCount = 100 * 50 * 50;
    appDesc->_maxRenderOpaqueInstanceCount = 100 * 50 * 50;

    cContext* context = new cContext();
    MyApp* app = new MyApp(context);
    app->Run();
    
    delete app;
    delete appDesc;

    return 0;
}*/