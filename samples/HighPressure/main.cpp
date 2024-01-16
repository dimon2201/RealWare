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
#include "disclaimer.hpp"
#include "intro.hpp"
#include "main_menu.hpp"

using namespace realware;

class MyApp : public core::cApplication
{

public:
    MyApp(const core::sApplicationDescriptor& desc) : core::cApplication(desc) {}
    ~MyApp() {}

    virtual void Init() override final
    {
        // Initialize managers
        core::textureManager->Init(m_renderContext, 1920, 1080, 12);
        render::mRender::Init(m_renderContext, 65536, 65536, 65536, 65536, glm::vec2(m_desc.WindowDesc.Width, m_desc.WindowDesc.Height));
        font::mFont::Init(m_renderContext);
        sound::mSound::Init(m_soundContext);

        // Render target info
        const float windowAspect = (core::mUserInput::GetWindowSize().x / core::mUserInput::GetWindowSize().y);

        // Create scene
        m_scene = new cScene(65536);
        m_scene->AddGlobal("SoundContext", m_soundContext);

        // Create intro
        m_intro = new cIntro(m_renderContext, m_soundContext, m_rt);

        // Create disclaimer
        m_disclaimer = new cDisclaimer(m_renderContext, m_soundContext, m_rt);

        // Create main menu
        m_mainMenu = new cMainMenu(m_renderContext, m_soundContext, m_rt);

        // Start intro
        m_intro->Init();
    }

    virtual void Update() override final
    {
        static float deltaTime = 0.0f;
        static float lastTime = clock();

        mUserInput::PollEvents();
        mUserInput::Update();

        if (m_stage == 0)
        {
            m_intro->Update(deltaTime, this, &m_stage);
            if (m_stage == 1)
            {
                m_intro->Free();
                m_mainMenu->Init();
            }
        }
        else if (m_stage == 1)
        {
            m_disclaimer->Update(deltaTime, this, &m_stage);
        }
        else if (m_stage == 2)
        {
            mRender::ClearRenderPass(mRender::GetOpaqueRenderPass(), K_TRUE, 0, glm::vec4(1.0f), K_TRUE, 1.0f);

            m_mainMenu->Update(deltaTime, this);
        }

        mRender::CompositeFinal();

        mUserInput::SwapBuffers();

        clock_t t = clock();

        deltaTime = t - lastTime;
        lastTime += deltaTime;
    }

    virtual void Free() override final
    {
        delete m_mainMenu;

        m_renderContext->DeleteRenderTarget(m_rt);
        delete m_rt;

        font::mFont::Free();
    }

private:
    cScene* m_scene;
    entity m_camera;
    render::sRenderTarget* m_rt;
    core::usize m_stage = 0;
    cDisclaimer* m_disclaimer;
    cMainMenu* m_mainMenu;
    cIntro* m_intro;

};

int main()
{
    core::sApplicationDescriptor appDesc;
    appDesc.WindowDesc.Title = "Test Window";
    appDesc.WindowDesc.Width = 1366;
    appDesc.WindowDesc.Height = 768;
    appDesc.WindowDesc.IsFullscreen = core::K_TRUE;

    MyApp app = MyApp(appDesc);
    app.Run();

    return 0;
}