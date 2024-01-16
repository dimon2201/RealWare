#pragma once

#include "types.hpp"

namespace realware
{
    namespace render
    {
        class cRenderContext;
        class cRenderer;
    }

    namespace sound
    {
        class cSoundContext;
    }

    namespace font
    {
        class cFontManager;
    }

    namespace core
    {
        class cUserInput;
        class cScene;
        class cCamera;
        class cTextureAtlas;

        struct sApplicationDescriptor
        {
            struct sWindowDescriptor
            {
                const char* Title;
                s32 Width;
                s32 Height;
                boolean IsFullscreen;
            };

            sWindowDescriptor WindowDesc;
        };

        class cApplication
        {

        public:
            explicit cApplication(const sApplicationDescriptor& desc);
            ~cApplication();

            void Run();

            virtual void Init() = 0;
            virtual void Update() = 0;
            virtual void Free() = 0;

        protected:
            sApplicationDescriptor m_desc;
            render::cRenderContext* m_renderContext;
            sound::cSoundContext* m_soundContext;

        };
    }
}