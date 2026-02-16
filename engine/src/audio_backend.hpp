// audio_backend.hpp

#pragma once

#include "audio.hpp"
#include "audio_backend_sound.hpp"
#include "backend.hpp"
#include "math.hpp"

namespace triton
{
    class cContext;

    class iAudioBackend : public iBackend
    {
        TRITON_OBJECT(iAudioBackend)

    public:
        explicit iAudioBackend(cContext* context) : iBackend(context) {}
        virtual ~iAudioBackend() = default;

        virtual sAudioBackendSound CreateSound(
            cAudio::eDataFormat dataFormat,
            const types::u8* data,
            types::usize dataByteSize,
            types::usize sampleRate
        ) = 0;
        virtual void DestroySound(sAudioBackendSound& sound) = 0;
        virtual void PlaySound(sAudioBackendSound& sound) = 0;
        virtual void StopSound(sAudioBackendSound& sound) = 0;
        virtual void SetSoundPosition(sAudioBackendSound& sound, const cVector3& position) = 0;
        virtual void SetSoundVelocity(sAudioBackendSound& sound, const cVector3& velocity) = 0;
        virtual void SetListenerPosition(const cVector3& position) = 0;
        virtual void SetListenerVelocity(const cVector3& velocity) = 0;
        virtual void SetListenerOrientation(const cVector3& at, const cVector3& up) = 0;
    };
}