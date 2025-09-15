#include "application.hpp"
#include "sound_manager.hpp"
#include "sound_context.hpp"
#include "memory_pool.hpp"

using namespace types;

namespace realware
{
    using namespace app;
    using namespace game;
    using namespace utils;

    namespace sound
    {
        cSound::cSound(const cApplication* const app, const u32 source, const u32 buffer) : _app((cApplication*)app), _source(source), _buffer(buffer)
        {
        }

        cSound::~cSound()
        {
            if (_file != nullptr)
            {
                if (_format == Category::SOUND_FORMAT_WAV)
                {
                    free(_file->Data);

                    _file->~sWAVStructure();
                    _app->GetMemoryPool()->Free(_file);
                }
            }
        }

        mSound::mSound(const cApplication* const app, const cSoundContext* const context) :
            _app((cApplication*)app), _context((cSoundContext*)context), _sounds((cApplication*)app, ((cApplication*)app)->GetDesc()->MaxSoundCount)
        {
        }

        cSound* mSound::AddSound(const std::string& id, const std::string& filename, const game::Category& format)
        {
            u32 source = 0;
            u32 buffer = 0;
            sWAVStructure* file = nullptr;
            _context->Create(filename, format, (const sWAVStructure** const)&file, source, buffer);

            return _sounds.Add(id, _app, source, buffer);
        }

        cSound* mSound::FindSound(const std::string& id)
        {
            return _sounds.Find(id);
        }

        void mSound::DeleteSound(const std::string& id)
        {
            _sounds.Delete(id);
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