#include <iostream>
#include <chrono>
#include <ctime>
#include <vector>

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
#include "../../engine/src/memory_pool.hpp"

using namespace realware::app;
using namespace realware::game;
using namespace realware::render;
using namespace realware::font;
using namespace realware::sound;
using namespace realware::physics;
using namespace realware::utils;
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

    sRenderPass* CreateCustomRenderPass()
    {
        mRender* render = GetRenderManager();
        mTexture* texture = GetTextureManager();
        iRenderContext* renderContext = GetRenderContext();
        sRenderPass* opaqueRenderPass = render->GetOpaqueRenderPass();
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
        sTextureAtlasTexture* customRenderPassTexture1 = texture->AddTexture(
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
        sTextureAtlasTexture* customRenderPassTexture2 = texture->AddTexture(
            "CustomRenderPassTexture2",
            glm::vec2(2, 2),
            4,
            &customRenderPassTexture2Data[0]
        );

        std::string vertexFunc = "";
        std::string fragmentFunc = "";
        render->LoadShaderFiles(
            "C:/DDD/RealWare/build_vs/samples/Sample01/Debug/data/shaders/custom_vertex.shader",
            "C:/DDD/RealWare/build_vs/samples/Sample01/Debug/data/shaders/custom_fragment.shader",
            vertexFunc,
            fragmentFunc
        );

        sRenderPass::sDescriptor renderPassDesc;
        renderPassDesc.InputVertexFormat = Category::VERTEX_BUFFER_FORMAT_POS_TEX_NRM_VEC3_VEC2_VEC3;
        renderPassDesc.InputBuffers.emplace_back(render->GetVertexBuffer());
        renderPassDesc.InputBuffers.emplace_back(render->GetIndexBuffer());
        renderPassDesc.InputBuffers.emplace_back(render->GetOpaqueInstanceBuffer());
        renderPassDesc.InputBuffers.emplace_back(render->GetOpaqueMaterialBuffer());
        renderPassDesc.InputBuffers.emplace_back(render->GetLightBuffer());
        renderPassDesc.InputBuffers.emplace_back(render->GetOpaqueTextureAtlasTexturesBuffer());
        renderPassDesc.InputTextures.emplace_back(GetTextureManager()->GetAtlas());
        renderPassDesc.InputTextureNames.emplace_back("TextureAtlas");
        renderPassDesc.InputTextureAtlasTextures.emplace_back(customRenderPassTexture1);
        renderPassDesc.InputTextureAtlasTextures.emplace_back(customRenderPassTexture2);
        renderPassDesc.InputTextureAtlasTextureNames.emplace_back("MyRedTexture");
        renderPassDesc.InputTextureAtlasTextureNames.emplace_back("MyWhiteTexture");
        renderPassDesc.ShaderBase = opaqueRenderPass->Desc.Shader;
        renderPassDesc.ShaderVertexFunc = vertexFunc;
        renderPassDesc.ShaderFragmentFunc = fragmentFunc;
        renderPassDesc.RenderTarget = opaqueRenderPass->Desc.RenderTarget;
        renderPassDesc.Viewport = glm::vec4(0.0f, 0.0f, windowSize.x, windowSize.y);
        renderPassDesc.DepthMode.UseDepthTest = K_TRUE;
        renderPassDesc.DepthMode.UseDepthWrite = K_TRUE;
        renderPassDesc.BlendMode.FactorCount = 1;
        renderPassDesc.BlendMode.SrcFactors[0] = sBlendMode::eFactor::ONE;
        renderPassDesc.BlendMode.DstFactors[0] = sBlendMode::eFactor::ZERO;
        
        return renderContext->CreateRenderPass(renderPassDesc);
    }

    virtual void Start() override final
    {
        // Create custom render pass
        _customRenderPass = CreateCustomRenderPass();

        // Triangle geometry
        _trianglePrimitive = _render->CreatePrimitive(Category::PRIMITIVE_TRIANGLE);
        _triangleGeometry = _render->CreateGeometry(
            _trianglePrimitive->Format,
            _trianglePrimitive->VerticesByteSize,
            _trianglePrimitive->Vertices,
            _trianglePrimitive->IndicesByteSize,
            _trianglePrimitive->Indices
        );
        _quadPrimitive = _render->CreatePrimitive(Category::PRIMITIVE_QUAD);
        _quadGeometry = _render->CreateGeometry(
            _quadPrimitive->Format,
            _quadPrimitive->VerticesByteSize,
            _quadPrimitive->Vertices,
            _quadPrimitive->IndicesByteSize,
            _quadPrimitive->Indices
        );

        // Models
        _cubeModel = _render->CreateModel("C:/DDD/RealWare/resources/cube.fbx");
        _cubeGeometry = _render->CreateGeometry(
            _cubeModel->Format,
            _cubeModel->VerticesByteSize,
            _cubeModel->Vertices,
            _cubeModel->IndicesByteSize,
            _cubeModel->Indices
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
        sMaterial* material1 = _render->AddMaterial(
            "Material1",
            texture1,
            glm::vec4(0.0f, 1.0f, 0.0f, 1.0f),
            glm::vec4(1.0f)
        );
        sMaterial* material2 = _render->AddMaterial(
            "Material2",
            texture2,
            glm::vec4(1.0f, 0.0f, 0.0f, 0.5f),
            glm::vec4(1.0f)
        );
        
        // Text
        sFont* font = _font->CreateFontTTF("C:/DDD/RealWare/resources/fonts/brahms.ttf", 64);
        sText* text = _font->CreateText(font, "New line\ntest\nanother newline\there");

        // Physics
        sSimulationScene* pxScene = _physics->AddScene("PXScene1");
        sSubstance* pxSubstance = _physics->AddSubstance("PXSubstance1");

        // Game objects
        const usize N = 30;
        for (usize z = 0; z < N; z++)
        {
            for (usize y = 0; y < N; y++)
            {
                for (usize x = 0; x < N; x++)
                {
                    const glm::vec3 position = glm::vec3(x, y, z);
                    const std::string id = "Cube" + std::to_string(x) + std::to_string(y) + std::to_string(z);

                    cGameObject* cubeObject1 = _gameObject->AddGameObject(id);
                    cubeObject1->SetVisible(K_TRUE);
                    cubeObject1->SetOpaque(K_FALSE);
                    cubeObject1->SetGeometry(_cubeGeometry);
                    cubeObject1->SetPosition(position);
                    cubeObject1->SetScale(glm::vec3(1.0f));
                    cubeObject1->SetMaterial(material2);
                }
            }
        }

        /*cGameObject* triangleObject1 = _gameObject->AddGameObject("TriangleObject1");
        triangleObject1->SetVisible(K_TRUE);
        triangleObject1->SetOpaque(K_TRUE);
        triangleObject1->SetGeometry(_triangleGeometry);
        triangleObject1->SetPosition(glm::vec3(0.0f, 0.0f, -1.0f));
        triangleObject1->SetScale(glm::vec3(1.0f));
        triangleObject1->SetMaterial(material1);

        cGameObject* triangleObject2 = _gameObject->AddGameObject("TriangleObject2");
        triangleObject2->SetVisible(K_TRUE);
        triangleObject2->SetOpaque(K_FALSE);
        triangleObject2->SetGeometry(_quadGeometry);
        triangleObject2->SetPosition(glm::vec3(0.0f, 0.0f, -3.0f));
        triangleObject2->SetScale(glm::vec3(1.0f));
        triangleObject2->SetMaterial(material2);

        cGameObject* cubeObject1 = _gameObject->AddGameObject("CubeObject1");
        cubeObject1->SetVisible(K_TRUE);
        cubeObject1->SetOpaque(K_TRUE);
        cubeObject1->SetGeometry(_cubeGeometry);
        cubeObject1->SetPosition(glm::vec3(0.0f, 3.0f, 0.0f));
        cubeObject1->SetScale(glm::vec3(1.0f));
        cubeObject1->SetMaterial(material1);
        cubeObject1->SetPhysicsActor(
            Category::PHYSICS_ACTOR_DYNAMIC,
            Category::PHYSICS_SHAPE_BOX,
            pxScene,
            pxSubstance,
            1000.0f
        );

        cGameObject* textObject = _gameObject->AddGameObject("TextObject");
        textObject->SetVisible(K_TRUE);
        textObject->SetOpaque(K_TRUE);
        textObject->SetPosition(glm::vec3(0.5f, 0.5f, 0.0f));
        textObject->SetScale(glm::vec3(1.0f));
        textObject->SetMaterial(material1);
        textObject->SetText(text);*/

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

        auto gameObjects = _gameObject->GetObjects();
        _opaqueGameObjects = new std::vector<cGameObject>();
        for (auto& gameObject : gameObjects.GetObjects())
        {
            if (gameObject.GetOpaque() == K_FALSE)
                _opaqueGameObjects->push_back(gameObject);
        }

        const auto& opaqueGameObjectsRef = *_opaqueGameObjects;
        _render->WriteObjectsToOpaqueBuffers(opaqueGameObjectsRef, _render->GetOpaqueRenderPass());
    }

    virtual void FrameUpdate() override final
    {
        // Custom render pass
        _customRenderPass->Desc.Viewport = glm::vec4(0.0f, 0.0f, GetWindowSize().x, GetWindowSize().y);

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
            _cubeGeometry,
            cameraObject
        );
        _render->CompositeFinal();
    }

    virtual void Finish() override final
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
    std::vector<cGameObject>* _opaqueGameObjects;
};

int main()
{
    sApplicationDescriptor* appDesc = new sApplicationDescriptor;
    appDesc->WindowDesc.Title = "Test Window";
    appDesc->WindowDesc.Width = 640;
    appDesc->WindowDesc.Height = 480;
    appDesc->WindowDesc.IsFullscreen = K_FALSE;
    appDesc->MemoryPoolByteSize = 64 * 1024 * 1024;

    appDesc->MaxGameObjectCount = 50 * 50 * 50;
    appDesc->MaxRenderOpaqueInstanceCount = 50 * 50 * 50;

    MyApp* app = new MyApp(appDesc);
    app->Run();
    
    delete app;
    delete appDesc;

    return 0;
}