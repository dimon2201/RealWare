#pragma once

#include "../../engine/src/core/ecs.hpp"

namespace realware
{
    namespace render
    {
        class cRenderContext;
        struct sRenderTarget;
        struct sRenderPass;
    }

    namespace sound
    {
        class cSoundContext;
    }
}

using namespace realware::core;
using namespace realware::render;
using namespace realware::sound;

class cIntro
{

public:
    cIntro(const cRenderContext* renderContext, const cSoundContext* soundContext, const sRenderTarget* renderTarget);
    ~cIntro();

    void Init();
    void Update(float frameTime, cApplication* application, usize* stage);
    void Free();

    inline cScene* GetScene() { return m_scene; }

private:
    cRenderContext* m_renderContext;
    cSoundContext* m_soundContext;
    cScene* m_scene;
    entity m_camera;
    sVertexBufferGeometry* m_quadGeometry;
    entity m_ddd;
    entity m_dollar;
    entity m_writing;

};