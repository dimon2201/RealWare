#pragma once

#include <vector>
#include <string>
#include "../../thirdparty/glm/glm/glm.hpp"
#include "sound_context.hpp"
#include "types.hpp"

namespace realware
{
    namespace app
    {
        class cApplication;
    }

    namespace sound
    {
        class mSound
        {
        public:
            mSound(const app::cApplication* const app, const cSoundContext* const context);
            ~mSound() = default;

            sSound* Load(const std::string& filename, const sSound::eFormat& format, const std::string& tag);
            void Remove(const std::string& tag);
            //void Play(core::entity object, core::cScene* scene);
            //void Stop(core::entity object, core::cScene* scene);

        private:
            app::cApplication* _app = nullptr;
            cSoundContext* _context = nullptr;
            std::vector<sSound*> _sounds = {};
        };
    }
}