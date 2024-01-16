#pragma once

#include "../../engine/src/core/ecs.hpp"

namespace realware
{
    namespace render
    {
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
using namespace realware::sound;
using namespace realware::font;

class cDisclaimer
{

public:
    cDisclaimer(const cRenderContext* renderContext, const cSoundContext* soundContext, const sRenderTarget* renderTarget);
    ~cDisclaimer();

    void Init();
    void Update(float frameTime, cApplication* application, usize* stage);
    void Free();

    inline cScene* GetScene() { return m_scene; }

private:
    cRenderContext* m_renderContext;
    cSoundContext* m_soundContext;
    cScene* m_scene;
    entity m_camera;
    sFont* m_font;
    entity m_beat;
    entity m_text;

};