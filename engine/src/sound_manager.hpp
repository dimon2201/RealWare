#pragma once

#include <vector>
#include <string>
#include "../../thirdparty/glm/glm/glm.hpp"
#include "category.hpp"
#include "types.hpp"

namespace realware
{
    namespace app
    {
        class cApplication;
    }

    namespace sound
    {
        struct sSound;
        class cSoundContext;

        class mSound
        {
        public:
            mSound(const app::cApplication* const app, const cSoundContext* const context);
            ~mSound() = default;

            sSound* Load(const std::string& filename, const game::Category& format, const std::string& tag);
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