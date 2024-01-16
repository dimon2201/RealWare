#include "intro.hpp"
#include "../../engine/src/core/user_input_manager.hpp"
#include "../../engine/src/render/render_manager.hpp"
#include "../../engine/src/render/render_context.hpp"
#include "../../engine/src/font/font_manager.hpp"
#include "../../engine/src/sound/sound_context.hpp"
#include "../../engine/src/sound/sound_manager.hpp"

cIntro::cIntro(const cRenderContext* renderContext, const cSoundContext* soundContext, const sRenderTarget* renderTarget)
{
    m_renderContext = (cRenderContext*)renderContext;
    m_soundContext = (cSoundContext*)soundContext;

    // Create scene
    m_scene = new cScene(65536);
    m_scene->AddGlobal("SoundContext", m_soundContext);

    // Create camera
    m_camera = m_scene->CreateEntity("Camera");
    m_scene->Add<sCCamera>(m_camera);

    // Sounds
    m_ddd = m_scene->CreateEntity("IntroDDD");
    sCTransform* dddTransform = m_scene->Add<sCTransform>(m_ddd);
    sCSound* dddSound = m_scene->Add<sCSound>(m_ddd);
    dddTransform->Init(K_TRUE, glm::vec3(0.0f, 0.25f, 0.0f), glm::vec3(0.0f), glm::vec3(0.5f, 0.4f, 1.0f));
    dddSound->Sound = mSound::LoadSound("C:\\Users\\USER100\\CLionProjects\\RealWare\\resources\\sounds\\ddd.wav", sSound::eFormat::WAV, "IntroDDD");

    // Quad geometry
    sPrimitive* quad = mRender::CreateQuad();
    m_quadGeometry = mRender::AddGeometry(
        quad->Format,
        quad->VerticesByteSize,
        quad->Vertices,
        quad->IndicesByteSize,
        quad->Indices
    );
    mRender::FreePrimitive(quad);

    // Textures
    textureManager->LoadTexture("C:\\Users\\USER100\\CLionProjects\\RealWare\\resources\\intro\\dollar.png", "IntroDollar");
    textureManager->LoadTexture("C:\\Users\\USER100\\CLionProjects\\RealWare\\resources\\intro\\writing.png", "IntroWriting");

    // Dollar
    m_dollar = m_scene->CreateEntity("Dollar");
    sCTransform* dollarTransform = m_scene->Add<sCTransform>(m_dollar);
    sCMaterial* dollarMaterial = m_scene->Add<sCMaterial>(m_dollar);
    sCGeometry* dollarGeometry = m_scene->Add<sCGeometry>(m_dollar);
    sCGeometryInfo* dollarGeometryInfo = m_scene->Add<sCGeometryInfo>(m_dollar);
    dollarTransform->Init(K_TRUE, glm::vec3(0.0f, 0.25f, 0.0f), glm::vec3(0.0f), glm::vec3(0.5f, 0.4f, 1.0f));
    dollarMaterial->Init(textureManager->GetTexture("IntroDollar"), glm::vec4(1.0f));
    dollarGeometry->Geometry = m_quadGeometry;
    dollarGeometryInfo->IsVisible = K_TRUE;
    dollarGeometryInfo->IsOpaque = K_TRUE;

    // Writing
    m_writing = m_scene->CreateEntity("Writing");
    sCTransform* writingTransform = m_scene->Add<sCTransform>(m_writing);
    sCMaterial* writingMaterial = m_scene->Add<sCMaterial>(m_writing);
    sCGeometry* writingGeometry = m_scene->Add<sCGeometry>(m_writing);
    sCGeometryInfo* writingGeometryInfo = m_scene->Add<sCGeometryInfo>(m_writing);
    writingTransform->Init(K_TRUE, glm::vec3(0.0f, -0.4, 0.0f), glm::vec3(0.0f), glm::vec3(0.4f, 0.15f, 1.0f));
    writingMaterial->Init(textureManager->GetTexture("IntroWriting"), glm::vec4(1.0f));
    writingGeometry->Geometry = m_quadGeometry;
    writingGeometryInfo->IsVisible = K_FALSE;
    writingGeometryInfo->IsOpaque = K_TRUE;
}

cIntro::~cIntro()
{
    textureManager->RemoveTexture("IntroDollar");
    textureManager->RemoveTexture("IntroWriting");
}

void cIntro::Init()
{
    mSound::PlaySound(m_ddd, m_scene);
}

void cIntro::Update(float frameTime, cApplication* application, usize* stage)
{
    static float counter = 0.0f;
    counter += 1.0f * (frameTime + 0.001f);
    float factor = 3650.0f;
    if (counter > factor)
    {
        sCGeometryInfo* writingGeometryInfo = m_scene->Get<sCGeometryInfo>(m_writing);
        writingGeometryInfo->IsVisible = K_TRUE;
    }

    mRender::SetCamera(m_camera);
    mRender::ClearRenderPasses(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 1.0f);
    mRender::DrawGeometryOpaque(
        application,
        m_quadGeometry,
        m_scene
    );

    if (counter >= 10500.0f)
    {
        Free();
        *stage = 1;
    }
}

void cIntro::Free()
{
    mSound::StopSound(m_ddd, m_scene);
}