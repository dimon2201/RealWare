// sound_context.hpp

#pragma once

#include <vector>
#include <AL/al.h>
#include <AL/alc.h>
#include "audio.hpp"
#include "category.hpp"
#include "object.hpp"
#include "math.hpp"
#include "types.hpp"

namespace triton
{
    struct sSound;
    struct sWAVStructure;
    class cApplication;
    class cContext;

    class cAudioBackendSound : public iObject
    {
        TRITON_OBJECT(cAudioBackendSound)

        ALuint _source = 0;
        ALuint _buffer = 0;

    public:
        explicit cAudioBackendSound(cContext* context, ALuint source, ALuint buffer)
            : iObject(context), _source(source), _buffer(buffer) {}
        virtual ~cAudioBackendSound() override final = default;

        inline ALuint Source() const { return _source; }
        inline ALuint Buffer() const { return _buffer; }
    };

    class iAudioBackend : public iObject
    {
        TRITON_OBJECT(iAudioBackend)

    public:
        explicit iAudioBackend(cContext* context) : iObject(context) {}
        virtual ~iAudioBackend() = default;

        virtual void CreateSound(sSound& sound) = 0;
        virtual void DestroySound(sSound& sound) = 0;
        virtual void PlaySound(sSound& sound) = 0;
        virtual void StopSound(sSound& sound) = 0;
        virtual void SetSoundPosition(sSound& sound, const cVector3& position) = 0;
        virtual void SetSoundVelocity(sSound& sound, const cVector3& velocity) = 0;
        virtual void SetListenerPosition(const cVector3& position) = 0;
        virtual void SetListenerVelocity(const cVector3& velocity) = 0;
        virtual void SetListenerOrientation(const cVector3& at, const cVector3& up) = 0;
    };

    class cOALAudioBackend : public iAudioBackend
    {
        TRITON_OBJECT(cOALAudioBackend)

        ALCdevice* _alDevice = nullptr;
        ALCcontext* _alContext = nullptr;

    public:
        explicit cOALAudioBackend(cContext* context);
        virtual ~cOALAudioBackend() override final;

        virtual void CreateSound(sSound& sound) override final;
        virtual void DestroySound(sSound& sound) override final;
        virtual void PlaySound(sSound& sound) override final;
        virtual void StopSound(sSound& sound) override final;
        virtual void SetSoundPosition(sSound& sound, const cVector3& position) override final;
        virtual void SetSoundVelocity(sSound& sound, const cVector3& velocity) override final;
        virtual void SetListenerPosition(const cVector3& position) override final;
        virtual void SetListenerVelocity(const cVector3& velocity) override final;
        virtual void SetListenerOrientation(const cVector3& at, const cVector3& up) override final;
    };
}