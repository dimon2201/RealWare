#pragma once

#include "../../engine/src/core/ecs.hpp"

namespace realware
{
    namespace render
    {
        struct sGeometry;
        struct sRenderTarget;
        struct sRenderPass;
        class cRenderContext;
    }

    namespace font
    {
        struct sFont;
    }

    namespace sound
    {
        class cSoundContext;
    }
}

using namespace realware::core;
using namespace realware::render;
using namespace realware::font;
using namespace realware::sound;

class cMainMenu
{

public:
    cMainMenu(const cRenderContext* renderContext, const cSoundContext* soundContext, const sRenderTarget* renderTarget);
    ~cMainMenu();

    void Init();
    void Update(float frameTime, cApplication* application);
    void Free();

    inline cScene* GetScene() { return m_scene; }

private:
    cRenderContext* m_renderContext;
    cSoundContext* m_soundContext;
    cScene* m_scene;
    entity m_camera;
    sFont* m_font;
    entity m_newGameButton;
    entity m_loadGameButton;
    entity m_optionsButton;
    entity m_exitButton;
    sVertexBufferGeometry* m_quadGeometry;
    std::vector<sArea*> m_backgroundAnimations[2];
    entity m_background[2];
    entity m_stain;
    entity m_splash;
    entity m_ambient;

    entity m_button;
    entity m_popup;
    entity m_check;

};