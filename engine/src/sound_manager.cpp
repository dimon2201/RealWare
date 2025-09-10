#include "sound_manager.hpp"
#include "sound_context.hpp"

namespace realware
{
    using namespace core;

    namespace sound
    {
        mSound::mSound(const core::cApplication* const app, const cSoundContext* const context) : _app((cApplication*)app), _context((cSoundContext*)context) {}

        sSound* mSound::Load(const std::string& filename, const sSound::eFormat& format, const std::string& tag)
        {
            sSound* sound = _context->Create(filename, format);
            sound->Tag = tag;

            _sounds.emplace_back(sound);

            return sound;
        }

        void mSound::Remove(const std::string& tag)
        {
            for (auto it = _sounds.begin(); it != _sounds.end(); it++)
            {
                if ((*it)->Tag == tag)
                {
                    _context->Destroy(*it);
                    _sounds.erase(it);
                    break;
                }
            }
        }

        /*void mSound::Play(entity object, cScene* scene)
        {
            core::sCSound* sound = scene->Get<core::sCSound>(object);
            core::sCTransform* transform = scene->Get<core::sCTransform>(object);
            _context->SetPosition(sound->Sound, transform->Position);
            _context->Play(sound->Sound);
        }

        void mSound::Stop(core::entity object, core::cScene* scene)
        {
            core::sCSound* sound = scene->Get<core::sCSound>(object);
            _context->Stop(sound->Sound);
        }*/
    }
}