#include "sound_manager.hpp"
#include "sound_context.hpp"
#include "ecs.hpp"

namespace realware
{
    namespace sound
    {
        mSound::mSound(cApplication* app, const cSoundContext* context)
        {
            m_app = app;
            m_context = (cSoundContext*)context;
        }

        mSound::~mSound()
        {
        }

        sSound* mSound::LoadSound(const char* filename, const sSound::eFormat& format, const std::string& tag)
        {
            sSound* sound = m_context->CreateSound(filename, format);
            sound->Tag = tag;

            m_sounds.emplace_back(sound);

            return sound;
        }

        void mSound::RemoveSound(const std::string& tag)
        {
            for (auto it = m_sounds.begin(); it != m_sounds.end(); it++)
            {
                if ((*it)->Tag == tag)
                {
                    m_context->FreeSound(*it);
                    m_sounds.erase(it);
                    break;
                }
            }
        }

        void mSound::PlaySound(core::entity object, core::cScene* scene)
        {
            core::sCSound* sound = scene->Get<core::sCSound>(object);
            core::sCTransform* transform = scene->Get<core::sCTransform>(object);
            m_context->SetSoundPosition(sound->Sound, transform->Position);
            m_context->PlaySound(sound->Sound);
        }

        void mSound::StopSound(core::entity object, core::cScene* scene)
        {
            core::sCSound* sound = scene->Get<core::sCSound>(object);
            m_context->StopSound(sound->Sound);
        }
    }
}