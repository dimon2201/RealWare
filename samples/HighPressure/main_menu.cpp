#include <iostream>
#include "main_menu.hpp"
#include "../../engine/src/render/render_manager.hpp"
#include "../../engine/src/render/render_context.hpp"
#include "../../engine/src/font/font_manager.hpp"
#include "../../engine/src/sound/sound_context.hpp"
#include "../../engine/src/core/texture_manager.hpp"
#include "../../engine/src/sound/sound_manager.hpp"
#include "../../engine/src/ui/widget_manager.hpp"

std::vector<sArea*> m_testAnimations[3];

void popupOnUpdate(cApplication* app, cScene* scene, sCWidget* widget) {}
void popupOnHover(cApplication* app, cScene* scene, sCWidget* widget) {}
void popupOnClick(cApplication* app, cScene* scene, sCWidget* widget) {}
void popupOptionOnUpdate(cApplication* app, cScene* scene, sCWidget* widget) {}
void popupOptionOnHover(cApplication* app, cScene* scene, sCWidget* widget) {}
void popupOptionOnClick(cApplication* app, cScene* scene, sCWidget* widget) {}

auto popupOnUpdateFunc = std::function<void(cApplication*, cScene*, sCWidget*)>(popupOnUpdate);
auto popupOnHoverFunc = std::function<void(cApplication*, cScene*, sCWidget*)>(popupOnHover);
auto popupOnClickFunc = std::function<void(cApplication*, cScene*, sCWidget*)>(popupOnClick);
auto popupOptionOnUpdateFunc = std::function<void(cApplication*, cScene*, sCWidget*)>(popupOptionOnUpdate);
auto popupOptionOnHoverFunc = std::function<void(cApplication*, cScene*, sCWidget*)>(popupOptionOnHover);
auto popupOptionOnClickFunc = std::function<void(cApplication*, cScene*, sCWidget*)>(popupOptionOnClick);

cMainMenu::cMainMenu(const cRenderContext* renderContext, const cSoundContext* soundContext, const sRenderTarget* renderTarget)
{
    m_renderContext = (cRenderContext*)renderContext;
    m_soundContext = (cSoundContext*)soundContext;

    // Create scene
    m_scene = new cScene(1 * 1024 * 1024);
    m_scene->AddGlobal("SoundContext", m_soundContext);

    // Create camera
    m_camera = m_scene->CreateEntity("Camera");
    m_scene->Add<sCCamera>(m_camera);

    // Sounds
    m_scene->AddGlobal("InterfaceHoverSound", m_soundContext->CreateSound("C:\\Users\\USER100\\CLionProjects\\RealWare\\resources\\sounds\\hover3.wav", sSound::eFormat::WAV));

    // Ambient
    m_ambient = m_scene->CreateEntity("MainMenuAmbient");
    sCTransform* ambientTransform = m_scene->Add<sCTransform>(m_ambient);
    sCSound* ambientSound = m_scene->Add<sCSound>(m_ambient);
    ambientTransform->Init(K_TRUE, glm::vec3(0.0f, 0.25f, 0.0f), glm::vec3(0.0f), glm::vec3(0.5f, 0.4f, 1.0f));
    ambientSound->Sound = mSound::LoadSound("C:\\Users\\USER100\\CLionProjects\\RealWare\\resources\\sounds\\main_menu.wav", sSound::eFormat::WAV, "IntroDDD");

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

    // Textures and animations
    textureManager->LoadTexture("C:\\Users\\USER100\\CLionProjects\\RealWare\\resources\\menu\\stain.png", "MainMenuStain");
    textureManager->LoadTexture("C:\\Users\\USER100\\CLionProjects\\RealWare\\resources\\menu\\splash.png", "MainMenuSplash");
//    textureManager->LoadAnimation(
//        {
//            "C:\\Users\\USER100\\CLionProjects\\RealWare\\resources\\widgets\\widget_button.png",
//            "C:\\Users\\USER100\\CLionProjects\\RealWare\\resources\\widgets\\widget_button2.png",
//        },
//        "WidgetButtonAnim"
//    );
    textureManager->LoadAnimation(
        {
            "C:\\Users\\USER100\\CLionProjects\\RealWare\\resources\\w_button.png",
        },
        "TestButtonAnim",
        m_testAnimations[0]
    );
    textureManager->LoadAnimation(
        {
            "C:\\Users\\USER100\\CLionProjects\\RealWare\\resources\\w_popup.png",
        },
        "TestComboAnim",
        m_testAnimations[1]
    );
    textureManager->LoadAnimation(
        {
            "C:\\Users\\USER100\\CLionProjects\\RealWare\\resources\\w_checkbox1.png",
            "C:\\Users\\USER100\\CLionProjects\\RealWare\\resources\\w_checkbox2.png"
        },
        "TestCheckAnim",
        m_testAnimations[2]
    );
//    textureManager->LoadAnimation(
//        {
//            "C:\\Users\\USER100\\CLionProjects\\RealWare\\resources\\widgets\\widget_check1.png",
//            "C:\\Users\\USER100\\CLionProjects\\RealWare\\resources\\widgets\\widget_check2.png",
//        },
//        "WidgetCheckAnim"
//    );
//    textureManager->LoadAnimation(
//        {
//            "C:\\Users\\USER100\\CLionProjects\\RealWare\\resources\\widgets\\widget_slider_button.png"
//        },
//        "WidgetSliderButtonAnim"
//    );
//    textureManager->LoadAnimation(
//        {
//            "C:\\Users\\USER100\\CLionProjects\\RealWare\\resources\\widgets\\widget_slider.png"
//        },
//        "WidgetSliderAnim"
//    );

    // Backgrounds
    textureManager->LoadAnimation(
        {
            "C:\\Users\\USER100\\CLionProjects\\RealWare\\resources\\menu\\animation\\pic0.png",
            "C:\\Users\\USER100\\CLionProjects\\RealWare\\resources\\menu\\animation\\pic1.png",
            "C:\\Users\\USER100\\CLionProjects\\RealWare\\resources\\menu\\animation\\pic2.png",
            "C:\\Users\\USER100\\CLionProjects\\RealWare\\resources\\menu\\animation\\pic3.png",
//            "C:\\Users\\USER100\\CLionProjects\\RealWare\\resources\\menu\\animation\\pic4.png",
//            "C:\\Users\\USER100\\CLionProjects\\RealWare\\resources\\menu\\animation\\pic5.png",
//            "C:\\Users\\USER100\\CLionProjects\\RealWare\\resources\\menu\\animation\\pic6.png",
//            "C:\\Users\\USER100\\CLionProjects\\RealWare\\resources\\menu\\animation\\pic7.png",
//            "C:\\Users\\USER100\\CLionProjects\\RealWare\\resources\\menu\\animation\\pic8.png",
//            "C:\\Users\\USER100\\CLionProjects\\RealWare\\resources\\menu\\animation\\pic9.png",
//            "C:\\Users\\USER100\\CLionProjects\\RealWare\\resources\\menu\\animation\\pic10.png",
//            "C:\\Users\\USER100\\CLionProjects\\RealWare\\resources\\menu\\animation\\pic11.png",
//            "C:\\Users\\USER100\\CLionProjects\\RealWare\\resources\\menu\\animation\\pic12.png"
        },
        "MainMenuAnim1",
        m_backgroundAnimations[0]
    );

    textureManager->LoadAnimation(
        {
            "C:\\Users\\USER100\\CLionProjects\\RealWare\\resources\\menu\\animation\\pic6.png",
            "C:\\Users\\USER100\\CLionProjects\\RealWare\\resources\\menu\\animation\\pic7.png",
            "C:\\Users\\USER100\\CLionProjects\\RealWare\\resources\\menu\\animation\\pic8.png",
            "C:\\Users\\USER100\\CLionProjects\\RealWare\\resources\\menu\\animation\\pic9.png",
//            "C:\\Users\\USER100\\CLionProjects\\RealWare\\resources\\menu\\animation\\pic9.png",
//            "C:\\Users\\USER100\\CLionProjects\\RealWare\\resources\\menu\\animation\\pic10.png",
//            "C:\\Users\\USER100\\CLionProjects\\RealWare\\resources\\menu\\animation\\pic11.png",
//            "C:\\Users\\USER100\\CLionProjects\\RealWare\\resources\\menu\\animation\\pic12.png"
        },
        "MainMenuAnim2",
        m_backgroundAnimations[1]
    );

    m_background[0] = m_scene->CreateEntity("Background1");
    sCTransform* backgroundTransform1 = m_scene->Add<sCTransform>(m_background[0]);
    sCMaterial* backgroundMaterial1 = m_scene->Add<sCMaterial>(m_background[0]);
    sCGeometry* backgroundGeometry1 = m_scene->Add<sCGeometry>(m_background[0]);
    sCGeometryInfo* backgroundGeometryInfo1 = m_scene->Add<sCGeometryInfo>(m_background[0]);
    sCAnimation* backgroundAnimation1 = m_scene->Add<sCAnimation>(m_background[0]);
    backgroundTransform1->Init(K_TRUE, glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f), glm::vec3(1.0f));
    backgroundMaterial1->Init(nullptr, glm::vec4(1.0f));
    backgroundGeometry1->Geometry = m_quadGeometry;
    backgroundGeometryInfo1->IsVisible = K_TRUE;
    backgroundGeometryInfo1->IsOpaque = K_FALSE;
    backgroundAnimation1->MaxTick = 1500.0f;
    backgroundAnimation1->Fade = 0.0f;
    backgroundAnimation1->Frames[0] = &m_backgroundAnimations[0];

    m_background[1] = m_scene->CreateEntity("Background2");
    sCTransform* backgroundTransform2 = m_scene->Add<sCTransform>(m_background[1]);
    sCMaterial* backgroundMaterial2 = m_scene->Add<sCMaterial>(m_background[1]);
    sCGeometry* backgroundGeometry2 = m_scene->Add<sCGeometry>(m_background[1]);
    sCGeometryInfo* backgroundGeometryInfo2 = m_scene->Add<sCGeometryInfo>(m_background[1]);
    sCAnimation* backgroundAnimation2 = m_scene->Add<sCAnimation>(m_background[1]);
    backgroundTransform2->Init(K_TRUE, glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f), glm::vec3(1.0f));
    backgroundMaterial2->Init(nullptr, glm::vec4(1.0f));
    backgroundGeometry2->Geometry = m_quadGeometry;
    backgroundGeometryInfo2->IsVisible = K_TRUE;
    backgroundGeometryInfo2->IsOpaque = K_TRUE;
    backgroundAnimation2->MaxTick = 1500.0f;
    backgroundAnimation2->Fade = 1.0f;
    backgroundAnimation2->Frames[0] = &m_backgroundAnimations[1];

    // Font
    m_font = mFont::CreateFont("C:\\Users\\USER100\\CLionProjects\\RealWare\\resources\\fonts\\tl.ttf", 64, 0, 0, 0);

    // Main menu buttons
    m_newGameButton = m_scene->CreateEntity("NewGameButton");
    sCTransform* newGameButtonTransform = m_scene->Add<sCTransform>(m_newGameButton);
    sCMaterial* newGameButtonMaterial = m_scene->Add<sCMaterial>(m_newGameButton);
    sCText* newGameButtonText = m_scene->Add<sCText>(m_newGameButton);
    sCButton* newGameButtonWidget = m_scene->Add<sCButton>(m_newGameButton);
    newGameButtonText->Font = m_font;
    newGameButtonText->Text = mFont::CyrillicStringInit("НОВАЯ");
    newGameButtonTransform->Init(K_TRUE, glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f), glm::vec3(1.0f));
    glm::vec3 newGameButtonWidgetPosition = newGameButtonTransform->Position;
    newGameButtonMaterial->Init(nullptr, glm::vec4(1.0f));
    newGameButtonWidget->Init(sCWidget::eType::BUTTON);
    newGameButtonWidget->OnUpdate = [newGameButtonWidgetPosition](cApplication* app_, cScene* scene_, sCWidget* widget_){
        sCTransform *transform = scene_->Get<sCTransform>(widget_->Owner);
        sCText *text = scene_->Get<sCText>(widget_->Owner);
        if (widget_->IsHovered == K_TRUE) {
            transform->Scale = glm::vec3(
                0.6f * mFont::GetTextWidth(text->Font, text->Text) * 1.1f,
                0.6f * mFont::GetTextHeight(text->Font, text->Text) * 1.1f,
                1.0f
            );
        } else {
            transform->Scale = glm::vec3(
                0.6f * mFont::GetTextWidth(text->Font, text->Text),
                0.6f * mFont::GetTextHeight(text->Font, text->Text),
                1.0f
            );
        }
        transform->Position.x = newGameButtonWidgetPosition.x - (0.25f * transform->Scale.x);
        transform->Position.y = newGameButtonWidgetPosition.y - (0.25f * transform->Scale.y);
    };
    newGameButtonWidget->OnClick = [](cApplication* app_, cScene* scene_, sCWidget* widget_){
    };
    newGameButtonWidget->OnHover = [](cApplication* app_, cScene* scene_, sCWidget* widget_){
    };

    m_loadGameButton = m_scene->CreateEntity("LoadGameButton");
    sCTransform* loadGameButtonTransform = m_scene->Add<sCTransform>(m_loadGameButton);
    sCMaterial* loadGameButtonMaterial = m_scene->Add<sCMaterial>(m_loadGameButton);
    sCText* loadGameButtonText = m_scene->Add<sCText>(m_loadGameButton);
    sCButton* loadGameButtonWidget = m_scene->Add<sCButton>(m_loadGameButton);
    loadGameButtonText->Font = m_font;
    loadGameButtonText->Text = mFont::CyrillicStringInit("ВСПОМНИТЬ");
    loadGameButtonTransform->Init(K_TRUE, glm::vec3(0.5f, 0.4f, 0.5f), glm::vec3(0.0f), glm::vec3(1.0f));
    glm::vec3 loadGameButtonWidgetPosition = loadGameButtonTransform->Position;
    loadGameButtonMaterial->Init(nullptr, glm::vec4(1.0f));
    loadGameButtonWidget->Init(sCWidget::eType::BUTTON);
    loadGameButtonWidget->OnUpdate = [loadGameButtonWidgetPosition](cApplication* app_, cScene* scene_, sCWidget* widget_){
        sCTransform *transform = scene_->Get<sCTransform>(widget_->Owner);
        sCText *text = scene_->Get<sCText>(widget_->Owner);
        if (widget_->IsHovered == K_TRUE) {
            transform->Scale = glm::vec3(
                0.6f * mFont::GetTextWidth(text->Font, text->Text) * 1.1f,
                0.6f * mFont::GetTextHeight(text->Font, text->Text) * 1.1f,
                1.0f
            );
        } else {
            transform->Scale = glm::vec3(
                0.6f * mFont::GetTextWidth(text->Font, text->Text),
                0.6f * mFont::GetTextHeight(text->Font, text->Text),
                1.0f
            );
        }
        transform->Position.x = loadGameButtonWidgetPosition.x - (0.25f * transform->Scale.x);
        transform->Position.y = loadGameButtonWidgetPosition.y - (0.25f * transform->Scale.y);
    };
    loadGameButtonWidget->OnClick = [](cApplication* app_, cScene* scene_, sCWidget* widget_){
    };
    loadGameButtonWidget->OnHover = [](cApplication* app_, cScene* scene_, sCWidget* widget_){
    };

    m_optionsButton = m_scene->CreateEntity("OptionsGameButton");
    sCTransform* optionsButtonTransform = m_scene->Add<sCTransform>(m_optionsButton);
    sCMaterial* optionsButtonMaterial = m_scene->Add<sCMaterial>(m_optionsButton);
    sCText* optionsButtonText = m_scene->Add<sCText>(m_optionsButton);
    sCButton* optionsButtonWidget = m_scene->Add<sCButton>(m_optionsButton);
    optionsButtonText->Font = m_font;
    optionsButtonText->Text = mFont::CyrillicStringInit("ОПЦИИ");
    optionsButtonTransform->Init(K_TRUE, glm::vec3(0.5f, 0.3f, 0.5f), glm::vec3(0.0f), glm::vec3(1.0f));
    glm::vec3 optionsButtonWidgetPosition = optionsButtonTransform->Position;
    optionsButtonMaterial->Init(nullptr, glm::vec4(1.0f));
    optionsButtonWidget->Init(sCWidget::eType::BUTTON);
    optionsButtonWidget->OnUpdate = [optionsButtonWidgetPosition](cApplication* app_, cScene* scene_, sCWidget* widget_){
        sCTransform *transform = scene_->Get<sCTransform>(widget_->Owner);
        sCText *text = scene_->Get<sCText>(widget_->Owner);
        if (widget_->IsHovered == K_TRUE) {
            transform->Scale = glm::vec3(
                0.6f * mFont::GetTextWidth(text->Font, text->Text) * 1.1f,
                0.6f * mFont::GetTextHeight(text->Font, text->Text) * 1.1f,
                1.0f
            );
        } else {
            transform->Scale = glm::vec3(
                0.6f * mFont::GetTextWidth(text->Font, text->Text),
                0.6f * mFont::GetTextHeight(text->Font, text->Text),
                1.0f
            );
        }
        transform->Position.x = optionsButtonWidgetPosition.x - (0.25f * transform->Scale.x);
        transform->Position.y = optionsButtonWidgetPosition.y - (0.25f * transform->Scale.y);
    };
    optionsButtonWidget->OnClick = [](cApplication* app_, cScene* scene_, sCWidget* widget_){
    };
    optionsButtonWidget->OnHover = [](cApplication* app_, cScene* scene_, sCWidget* widget_){
    };

    m_exitButton = m_scene->CreateEntity("OptionsGameButton");
    sCTransform* exitButtonTransform = m_scene->Add<sCTransform>(m_exitButton);
    sCMaterial* exitButtonMaterial = m_scene->Add<sCMaterial>(m_exitButton);
    sCText* exitButtonText = m_scene->Add<sCText>(m_exitButton);
    sCButton* exitButtonWidget = m_scene->Add<sCButton>(m_exitButton);
    exitButtonText->Font = m_font;
    exitButtonText->Text = mFont::CyrillicStringInit("ВЫЙТИ");
    exitButtonTransform->Init(K_TRUE, glm::vec3(0.5f, 0.2f, 0.5f), glm::vec3(0.0f), glm::vec3(1.0f));
    glm::vec3 exitButtonWidgetPosition = exitButtonTransform->Position;
    exitButtonMaterial->Init(nullptr, glm::vec4(1.0f));
    exitButtonWidget->Init(sCWidget::eType::BUTTON);
    exitButtonWidget->OnUpdate = [exitButtonWidgetPosition](cApplication* app_, cScene* scene_, sCWidget* widget_){
        sCTransform *transform = scene_->Get<sCTransform>(widget_->Owner);
        sCText *text = scene_->Get<sCText>(widget_->Owner);
        if (widget_->IsHovered == K_TRUE) {
            transform->Scale = glm::vec3(
                0.6f * mFont::GetTextWidth(text->Font, text->Text) * 1.1f,
                0.6f * mFont::GetTextHeight(text->Font, text->Text) * 1.1f,
                1.0f
            );
        } else {
            transform->Scale = glm::vec3(
                0.6f * mFont::GetTextWidth(text->Font, text->Text),
                0.6f * mFont::GetTextHeight(text->Font, text->Text),
                1.0f
            );
        }
        transform->Position.x = exitButtonWidgetPosition.x - (0.25f * transform->Scale.x);
        transform->Position.y = exitButtonWidgetPosition.y - (0.25f * transform->Scale.y);
    };
    exitButtonWidget->OnClick = [](cApplication* app_, cScene* scene_, sCWidget* widget_){
    };
    exitButtonWidget->OnHover = [](cApplication* app_, cScene* scene_, sCWidget* widget_){
    };

    // Stain
//    m_stain = m_scene->CreateEntity("Stain");
//    sCTransform* stainTransform = m_scene->Add<sCTransform>(m_stain);
//    sCMaterial* stainMaterial = m_scene->Add<sCMaterial>(m_stain);
//    sCGeometry* stainGeometry = m_scene->Add<sCGeometry>(m_stain);
//    sCGeometryInfo* stainGeometryInfo = m_scene->Add<sCGeometryInfo>(m_stain);
//    stainTransform->Init(K_TRUE, glm::vec3(0.005f, -0.28f, 0.0f), glm::vec3(0.0f), glm::vec3(0.45f, 0.7f, 1.0f));
//    stainMaterial->Init(textureManager->GetTexture("MainMenuStain"), glm::vec4(1.0f));
//    stainGeometry->Geometry = m_quadGeometry;
//    stainGeometryInfo->IsVisible = K_TRUE;
//    stainGeometryInfo->IsOpaque = K_TRUE;

    // Splash
    m_splash = m_scene->CreateEntity("Splash");
    sCTransform* splashTransform = m_scene->Add<sCTransform>(m_splash);
    sCMaterial* splashMaterial = m_scene->Add<sCMaterial>(m_splash);
    sCGeometry* splashGeometry = m_scene->Add<sCGeometry>(m_splash);
    sCGeometryInfo* splashGeometryInfo = m_scene->Add<sCGeometryInfo>(m_splash);
    splashTransform->Init(K_TRUE, glm::vec3(0.0f, 0.68f, 0.0f), glm::vec3(0.0f), glm::vec3(0.42f, 0.24f, 1.0f));
    splashMaterial->Init(textureManager->GetTexture("MainMenuSplash"), glm::vec4(1.0f));
    splashGeometry->Geometry = m_quadGeometry;
    splashGeometryInfo->IsVisible = K_TRUE;
    splashGeometryInfo->IsOpaque = K_FALSE;

    // Test popup
    {
        realware::ui::mWidget::sCreatePopupDescriptor desc;
        desc.Scene = m_scene;
        desc.Name = "TestPopup";
        desc.Position = glm::vec3(0.8f, 0.4f, 0.5f);
        desc.Scale = glm::vec2(0.25f, 0.1f);
        desc.Font = m_font;
        desc.TextScale = 0.6f;
        desc.PopupOnUpdate = popupOnUpdateFunc;
        desc.PopupOnHover = popupOnHoverFunc;
        desc.PopupOnClick = popupOnClickFunc;
        desc.PopupAnimation = &m_testAnimations[1];
        desc.PopupOptionTitles = {"Button1", "Button2"};
        desc.OptionOnUpdate = popupOptionOnUpdateFunc;
        desc.OptionOnHover = popupOptionOnHoverFunc;
        desc.OptionOnClick = popupOptionOnClickFunc;
        desc.OptionAnimation = &m_testAnimations[0];
        m_popup = realware::ui::mWidget::CreatePopupMenu(&desc);
    }
    {
        realware::ui::mWidget::sCreateCheckboxDescriptor desc;
        desc.Scene = m_scene;
        desc.Name = "TestCheck";
        desc.Position = glm::vec3(0.8f, 0.2f, 0.5f);
        desc.Scale = glm::vec2(0.08f, 0.08f);
        desc.Font = m_font;
        desc.TextScale = 0.6f;
        desc.Text = "Checkbox";
        desc.OnUpdate = [](cApplication* app_, cScene* scene_, sCWidget* widget_){};
        desc.OnHover = [](cApplication* app_, cScene* scene_, sCWidget* widget_){};
        desc.OnClick = [](cApplication* app_, cScene* scene_, sCWidget* widget_){
            sCAnimation* animation = scene_->Get<sCAnimation>(widget_->Owner);
            animation->CurrentFrameIndex[0] ^= 1;
            cSoundContext* context = (cSoundContext*)scene_->GetGlobal("SoundContext");
            context->PlaySound((const sSound*)scene_->GetGlobal("InterfaceHoverSound"));
        };
        desc.Animation = &m_testAnimations[2];
        m_check = realware::ui::mWidget::CreateCheckbox(&desc);
    }
}

cMainMenu::~cMainMenu()
{
    mRender::DeleteGeometry(m_quadGeometry);
    mFont::DeleteFont(m_font);
}

void cMainMenu::Init()
{
    mSound::PlaySound(m_ambient, m_scene);
}

void cMainMenu::Update(float frameTime, cApplication* application)
{
    mRender::SetCamera(m_camera);
    mRender::ClearRenderPasses(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), 1.0f);
    mRender::DrawGeometryOpaque(
        application,
        m_quadGeometry,
        m_scene
    );
    mRender::DrawGeometryTransparent(
        application,
        m_quadGeometry,
        m_scene
    );
    mRender::CompositeTransparent();

    realware::ui::mWidget::Update(application, m_scene);
//    mRender::DrawWidgets(application, m_scene);
    mRender::DrawTexts(application, m_scene);

    textureManager->PlayAnimation(m_background[0], m_scene, 1.0f);
    textureManager->PlayAnimation(m_background[1], m_scene, 1.0f);

    sCAnimation* anim0 = m_scene->Get<sCAnimation>(m_background[0]);
    sCAnimation* anim1 = m_scene->Get<sCAnimation>(m_background[1]);
    if (anim0->Tick > 2.0f * (anim0->MaxTick / 8.0f) && anim0->Tick < 3.0f * (anim0->MaxTick / 8.0f))
    {
        anim0->Fade += 1.0f / ((float)anim0->MaxTick / 8.0f);
        anim1->Fade -= 1.0f / ((float)anim0->MaxTick / 8.0f);
    }
    if (abs(anim0->Tick - (3.0f * (anim0->MaxTick / 8.0f))) < 1.0f)
    {
        textureManager->IncrementAnimationFrame(m_background[0], m_scene);
    }
    if (anim0->Tick > 6.0f * (anim0->MaxTick / 8.0f) && anim0->Tick < 7.0f * (anim0->MaxTick / 8.0f))
    {
        anim0->Fade -= 1.0f / ((float)anim0->MaxTick / 8.0f);
        anim1->Fade += 1.0f / ((float)anim0->MaxTick / 8.0f);
    }
    if (abs(anim0->Tick - (7.0f * (anim0->MaxTick / 8.0f))) < 1.0f)
    {
        textureManager->IncrementAnimationFrame(m_background[1], m_scene);
    }
}

void cMainMenu::Free()
{
    mSound::StopSound(m_ambient, m_scene);
}