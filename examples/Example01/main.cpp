#include <iostream>
#include "capabilities.hpp"
#include "context.hpp"
#include "application.hpp"
#include "graphics.hpp"
#include "vertex.hpp"
#include "camera.hpp"
#include "math.hpp"
#include "animation.hpp"
#include "camera_pool.hpp"
#include "geometry_storage.hpp"
#include "model3d_pool.hpp"
#include "skin_pool.hpp"
#include "render_pass_pools.hpp"
#include "material_pool.hpp"
#include "geometry_storage.hpp"
#include "world_object_pool.hpp"

using namespace triton;
using namespace types;

class cMyApplication final : public IApplication
{
public:
    cMyApplication(cContext* context, const sCapabilities& caps) : IApplication(context, caps)
    {
    }

    virtual ~cMyApplication() override final
    {
    }

    virtual void Setup() override final
    {
        CGeometryStorage* gs = _context->GetSubsystem<CGeometryStorage>();
        CGraphics* gfx = _context->GetSubsystem<CGraphics>();
        CMaterialPool* materialPool = _context->GetPool<CMaterialPool>();
        CWorldObjectPool* worldObjectPool = _context->GetPool<CWorldObjectPool>();
        CModel3DPool* model3DPool = _context->GetPool<CModel3DPool>();

        auto camh = *_context->GetPool<CCameraPool>()->Create();
        XCamera& camo = *_context->GetPool<CCameraPool>()->Get(camh);
        camo.SetWorldPosition(cVector3(0.0f, 0.0f, 30.0f));
        
        //opaqueStaticRP.SetCamera(camh);
        
        /*m3d = *_context->GetSubsystem<XModel3DSubsystem>()->CreateModelFromAsset(
            "C:/My/My_Projects_Programming/TritonEngine/tools/tasset/bin/Chort2.tasset"
        );
        HGameObject myObject1 = gos->CreateGameObject("MyObject1");
        gos->SetRenderableStatic(
            myObject1,
            m3d
        );*/

        SRigidVertexGPULayout triVerts[3];
        triVerts[0].position = cVector3(-1.0f, -1.0f, 0.0f);
        triVerts[1].position = cVector3(0.0f, 1.0f, 0.0f);
        triVerts[2].position = cVector3(1.0f, -1.0f, 0.0f);
        triVerts[0].texcoord = cVector2(0.0f, 0.0f);
        triVerts[1].texcoord = cVector2(0.0f, 1.0f);
        triVerts[2].texcoord = cVector2(1.0f, 0.0f);
        u32 triInds[3] = { 0, 1, 2 };

        SGeometryView triGeom = *gs->Create(
            EVertexBufferFormat::Rigid_48,
            (u8*)triVerts,
            3,
            (u8*)triInds,
            3
        );

        /*XMaterial::THandle material = *materialPool->Create(
            std::nullopt,
            std::nullopt,
            std::nullopt,
            std::nullopt,
            cVector4(1.0f),
            cVector4(1.0f),
            0.0f
        );*/
       
        //SVertex quadVerts[4];
        //quadVerts[0].position = cVector3(-1.0f, -1.0f, 0.0f);
        //quadVerts[1].position = cVector3(-1.0f, 1.0f, 0.0f);
        //quadVerts[2].position = cVector3(1.0f, 1.0f, 0.0f);
        //quadVerts[3].position = cVector3(1.0f, -1.0f, 0.0f);
        //u32 quadInds[6] = { 0, 1, 2, 0, 2, 3 };

        /*
            auto model = models->Load(
                "models/nathan/nathan_triton.fbx"
            );

            auto nathan = scene->CreateModel(model);

            nathan.SetName("MyChort1");
            nathan.SetTransform(...);

            nathan.PlayAnimation("Idle");
        */

        /*
            auto model = assetLoader->Load("models/nathan/nathan_triton.fbx");

            auto nathan = gameObjectSubsystem->Create(model);
            nathan->SetName("Nathan1");

            ...
            nathan->PlayAnimation("Idle");
        */

        //m3d = *_context->GetSubsystem<XModel3DSubsystem>()->CreateFromRaw(
        //    "C:/My/My_Projects_Programming/TritonEngine/runtime/data/models/nathan/nathan_triton.fbx"
        //);
        //auto m3dd = *_context->GetSubsystem<XModel3DSubsystem>()->GetPool()->Get(m3d);

        auto modelHandle = *_context->GetPool<CModel3DPool>()->Create(
            EVertexBufferFormat::Rigid_48,
            EModel3DFileType::Raw,
            "C:/My/My_Projects_Programming/TritonEngine/runtime/data/models/lighthouse/lighthouse.fbx"
        );
        XModel3D& model = *_context->GetPool<CModel3DPool>()->Get(modelHandle);

        SGeometryView modelGeom = *gs->Create(
            model.GetVertexFormat(),
            (u8*)model.GetRigidVertices(),
            model.GetVertexCount(),
            (u8*)model.GetIndices(),
            model.GetIndexCount()
        );

        /*auto gameObject1Handle = *gameObjectPool->Create("MyObject1");
        XGameObject& gameObject1 = *gameObjectPool->Get(gameObject1Handle);
        gameObject1.SetRenderable(
            True,
            instancePack1Handle
        );
        gameObject1.SetRotation(
            cVector3(-90.0f, 0.0f, 0.0f)
        );
        auto gameObject2Handle = *gameObjectPool->Create("MyObject2");
        XGameObject& gameObject2 = *gameObjectPool->Get(gameObject2Handle);
        gameObject2.SetRenderable(
            True,
            instancePack2Handle
        );
        gameObject2.SetWorldPosition(
            cVector3(0.0f, 128.0f, 0.0f)
        );
        gameObject2.SetRotation(
            cVector3(-90.0f, 0.0f, 0.0f)
        );*/

        /*const usize cGridSize = 2;
        const usize cGridStep = 256;
        SGeometryView modelGeom = *gs->Create(
            EVertexBufferFormat::Rigid_48,
            (u8*)model.GetRigidVertices(),
            model.GetVertexCount(),
            (u8*)model.GetIndices(),
            model.GetIndexCount()
        );
        XRenderInstancePack::THandle instancePack1Handle = *instancePackPool->Create(
            ERenderInstanceMotionType::Static,
            modelGeom,
            model.GetMaterial(),
            cGridSize * cGridSize * cGridSize
        );
        XTexture::THandle emptyTexHandle = XTexture::THandle();
        auto material1 = *materialPool->Create(
            emptyTexHandle,
            emptyTexHandle,
            emptyTexHandle,
            emptyTexHandle,
            cVector4(1.0f, 1.0f, 1.0f, 1.0f),
            cVector4(1.0f, 1.0f, 1.0f, 1.0f),
            1.0f
        );
        
        //opaqueStaticRP.SetRenderInstancePacks({ instancePack1Handle });

        XMaterial& modelMaterial = *_context->GetPool<CMaterialPool>()->Get(model.GetMaterial());
        modelMaterial.SetShininess(512.0f);

        for (s32 x = 0; x < cGridSize; x++)
        {
            for (s32 y = 0; y < cGridSize; y++)
            {
                for (s32 z = 0; z < cGridSize; z++)
                {
                    auto gameObjectHandle = *gameObjectPool->Create("MyObject");
                    XGameObject& gameObject = *gameObjectPool->Get(gameObjectHandle);
                    gameObject.SetRenderable(
                        True,
                        instancePack1Handle
                    );
                    gameObject.SetMaterial(model.GetMaterial());
                    gameObject.SetWorldPosition(
                        cVector3(
                            (x - s32(cGridSize / 2)) * (s32)cGridStep,
                            (y - s32(cGridSize / 2)) * (s32)cGridStep,
                            (z - s32(cGridSize / 2)) * (s32)cGridStep
                        )
                    );
                    gameObject.SetRotation(cVector3(-90.0f, 0.0f, 0.0f));
                }
            }
        }

        _context->GetSubsystem<CGraphics>()->SetShadingModel(EShadingModel::PBR);*/

        //triGod1.worldPosition = cVector3(-1.0f, 0.0f, 0.0f);
        //triGod1.worldRotation = cVector3(0.0f, 0.0f, 0.0f);
        //triGod1.scale = cVector3(0.25f * 0.5f);
        //sis.UpdateTransform(triRi1, _context->GetSubsystem<XGameObjectSubsystem>());

        /*HGameObject quadObj1 = gos->CreateGameObject("MyQuad1");
        SGameObjectData& quadGod1 = gos->Get(quadObj1);
        god = &quadGod1;
        HRenderInstance quadRi1 = gos->SetRenderable(
            quadObj1,
            ERenderInstanceMotionType::Dynamic,
            EGraphicsBufferFormat::POSITION_TEXCOORD_NORMAL_TANGENT_VEC3_VEC2_VEC3_VEC4,
            (u8*)&quadVerts[0],
            sizeof(SVertex) * 4,
            (u8*)&quadInds[0],
            sizeof(u32) * 6
        );
        ri = quadRi1;
        quadGod1.worldPosition = cVector3(1.0f, 0.0f, 0.0f);
        quadGod1.worldRotation = cVector3(0.0f, 0.0f, 0.0f);
        //quadGod1.scale = cVector3(0.5f * 0.5f);
        dis.UpdateTransform(quadRi1, _context->GetSubsystem<XGameObjectSubsystem>());

        HTexture t1 = ts->CreateTexture(
            "C:/My/My_Projects_Programming/TritonEngine/runtime/data/models/brick.png",
            cTexture::eFormat::RGBA8_SRGB_MIPS
        );
        HMaterial m1 = _context->GetSubsystem<XMaterialSubsystem>()->CreateMaterial(
            cVector4(1.0f),
            t1,
            {},
            {},
            {}
        );
        HGameObject triObj2 = gos->CreateGameObject("MyTriangle2");
        SGameObjectData& triGod2 = gos->Get(triObj2);
        SRenderInstanceData& triRid1 =
            _context->GetSubsystem<XBatchSubsystem>()->GetStaticInstanceStorage().Get(triRi1);
        HRenderInstance triRi2 = gos->SetRenderable(
            triObj2,
            ERenderInstanceMotionType::Static,
            EGraphicsBufferFormat::POSITION_TEXCOORD_NORMAL_TANGENT_VEC3_VEC2_VEC3_VEC4,
            (u8*)&triVerts[0],
            sizeof(SVertex) * 3,
            (u8*)&triInds[0],
            sizeof(u32) * 3,
            triRid1.batch,
            m1
        );
        triGod2.worldPosition = cVector3(3.0f, 0.0f, 0.0f);
        triGod2.worldRotation = cVector3(0.0f, 0.0f, 0.0f);
        //triGod2.scale = cVector3(0.75f * 0.5f);
        sis.UpdateTransform(triRi2, _context->GetSubsystem<XGameObjectSubsystem>());*/

        /*XTextureSubsystem* ts = _context->GetSubsystem<XTextureSubsystem>();
        HTexture t1 = ts->CreateTexture("C:/My/My_Projects_Programming/TritonEngine/runtime/data/textures/dirt.png");
        HTexture t2 = ts->CreateTexture("C:/My/My_Projects_Programming/TritonEngine/runtime/data/textures/brick.png");
        HTexture t2n = ts->CreateTexture("C:/My/My_Projects_Programming/TritonEngine/runtime/data/textures/brick_normal.png");
        
        XMaterialSubsystem* ms = _context->GetSubsystem<XMaterialSubsystem>();
        HMaterial m1 = ms->CreateMaterial(cVector4(1.0f), t1, t2n, {}, {});
        HMaterial m2 = ms->CreateMaterial(cVector4(1.0f), t2, t2n, {}, {});

        //gos->SetMaterial(go, m2);

        





        /*sChunkAllocatorDescriptor cad;
        cad.chunkByteSize = 1024;
        cad.hashTableSize = 4096;
        cad.maxChunkCount = 65536;
        da = new XDynamicArray<int>(_context, cad);
        ha = new XHandleAllocator<SSlot, HSkinnedBone, XDynamicArray<SSkinnedBoneData>, SSkinnedBoneData>(_context);
        ha->Initialize();*/
    }

    virtual void Update() override final
    {
        //XDynamicInstanceStorage& dis = _context->GetSubsystem<XBatchSubsystem>()->GetDynamicInstanceStorage();
        //god->worldPosition._vec.y += 0.01f;
        //god->worldRotation._vec.z += 0.01f;
        //dis.UpdateTransform(ri);

        /*auto h0 = ha->Create();
        ha->Destroy(h0);*/

        /*static f32 time = 0.0f;
        SModel3DData& modelData = *_context->GetSubsystem<XModel3DSubsystem>()->GetPool()->Get(m3d);
        SSkeletonData::THandle& skeleton = modelData.skeleton;
        SFrame frame = *_context->GetSubsystem<XAnimationSubsystem>()->Evaluate(
            skeleton,
            modelData.animations[0],
            time
        );
        _context->GetSubsystem<XSkinningSubsystem>()->Skin(
            m3dSkin,
            frame
        );
        time += 0.1f;*/

        //_context->GetSubsystem<XGameObjectSubsystem>()->PlayAnimation(goh, 0);
    }

    virtual void Stop() override final
    {
    }
};

int main()
{
    cContext* context = new cContext();
    std::cout << "Context initialized." << std::endl;

    sCapabilities caps = {};
    caps.graphicsBackend = EAvailableGraphicsBackend::Vulkan;
    caps.preferredGraphicsDevice = EGraphicsDeviceType::Discrete;
    caps.framesInFlight = 2;
    caps.window.title = "My Test Application";
    caps.window.size = cVector2(800, 600);
    caps.window.fullscreen = K_FALSE;
    caps.hashTableChunkByteSize = 1024;
    caps.staticVertexBufferSize = 70 * 1024 * 1024;
    caps.skinnedVertexBufferSize = 70 * 1024 * 1024;

    cMyApplication* myApp = new cMyApplication(context, caps);
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