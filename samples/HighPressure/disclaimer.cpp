#include "disclaimer.hpp"
#include "../../engine/src/core/types.hpp"
#include "../../engine/src/render/render_manager.hpp"
#include "../../engine/src/render/render_context.hpp"
#include "../../engine/src/font/font_manager.hpp"
#include "../../engine/src/sound/sound_context.hpp"
#include "../../engine/src/sound/sound_manager.hpp"

cDisclaimer::cDisclaimer(const cRenderContext* renderContext, const cSoundContext* soundContext, const sRenderTarget* renderTarget)
{
    m_renderContext = (cRenderContext*)renderContext;
    m_soundContext = (cSoundContext*)soundContext;

    // Create scene
    m_scene = new cScene(65536);

    // Create camera
    m_camera = m_scene->CreateEntity("Camera");
    m_scene->Add<sCCamera>(m_camera);

    // Font
    m_font = mFont::CreateFont("C:\\Users\\USER100\\CLionProjects\\RealWare\\resources\\fonts\\tl.ttf", 64, 0, 0.1f, 0.025f);

    // Beat
    m_beat = m_scene->CreateEntity("DisclaimerBeat");
    sCTransform* beatTransform = m_scene->Add<sCTransform>(m_beat);
    sCSound* beatSound = m_scene->Add<sCSound>(m_beat);
    beatTransform->Init(K_TRUE, glm::vec3(0.0f, 0.25f, 0.0f), glm::vec3(0.0f), glm::vec3(0.5f, 0.4f, 1.0f));
    beatSound->Sound = mSound::LoadSound("C:\\Users\\USER100\\CLionProjects\\RealWare\\resources\\sounds\\disclaimer.wav", sSound::eFormat::WAV, "IntroDDD");

    // Text
    m_text = m_scene->CreateEntity("DisclaimerText");
    sCTransform* textTransform = m_scene->Add<sCTransform>(m_text);
    sCMaterial* textMaterial = m_scene->Add<sCMaterial>(m_text);
    sCText* textText = m_scene->Add<sCText>(m_text);
    textTransform->Init(K_TRUE, glm::vec3(0.5f, 0.5f, 0.0f), glm::vec3(0.0f), glm::vec3(1.0f));
    textMaterial->Init(nullptr, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
    textText->Font = m_font;
    textText->Text = mFont::CyrillicStringInit("   ВСЕ СОБЫТИЯ И ПЕРСОНАЖИ В ДАННОЙ ВИДЕОИГРЕ ВЫМЫШЛЕННЫЕ.\nЛЮБОЕ СХОДСТВО С РЕАЛЬНЫМИ СОБЫТИЯМИ И ПЕРСОНАЖАМИ СЛУЧАЙНО,\n       АВТОР НЕ НЕСЕТ ОТВЕТСТВЕННОСТИ ЗА ПРИЧИНЕННЫЙ ВРЕД\n          СВОЕЙ ВИДЕОИГРОЙ. ВСЕ ПРЕДСТАВЛЕННОЕ ЯВЛЯЕТСЯ\n               ИСКЛЮЧИТЕЛЬНО ЛИЧНЫМ МНЕНИЕМ АВТОРА");
    textTransform->Scale = glm::vec3(mFont::GetTextWidth(textText->Font, textText->Text), 0.5f, 1.0f);
    textTransform->Position.x -= 0.25f * textTransform->Scale.x;
    textTransform->Position.y -= 0.25f * textTransform->Scale.y;
}

cDisclaimer::~cDisclaimer()
{
    mFont::DeleteFont(m_font);
}

void cDisclaimer::Init()
{
    mSound::PlaySound(m_beat, m_scene);
}

void cDisclaimer::Update(float frameTime, cApplication* application, usize* stage)
{
    static float counter = 0.0f;
    counter += 1.0f * (frameTime + 0.001f);

    mRender::SetCamera(m_camera);
    mRender::ClearRenderPasses(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), 1.0f);
    mRender::DrawTexts(application, m_scene);

    if (counter >= 10000.0f) {
        *stage = 2;
    }
}

void cDisclaimer::Free()
{
    mSound::StopSound(m_beat, m_scene);
}