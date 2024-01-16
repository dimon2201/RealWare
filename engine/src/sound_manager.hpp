#pragma once

#include <vector>
#include <string>
#include "../../thirdparty/glm/glm/glm.hpp"
#include "sound_context.hpp"
#include "ecs.hpp"
#include "types.hpp"

namespace realware
{
    namespace core
    {
        class cApplication;
    }

    namespace sound
    {
        class mSound
        {

        public:
            mSound() {}
            ~mSound() {}

            void Init(const cSoundContext* context);
            void Free();
            sSound* LoadSound(const char* filename, const sSound::eFormat& format, const std::string& tag);
            void RemoveSound(const std::string& tag);
            void PlaySound(core::entity object, core::cScene* scene);
            void StopSound(core::entity object, core::cScene* scene);

        private:
            cSoundContext* m_context = nullptr;
            std::vector<sSound*> m_sounds;

        };
    }
}