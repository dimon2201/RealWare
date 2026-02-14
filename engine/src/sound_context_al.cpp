// sound_context_al.cpp

#include <iostream>
#include <cstdio>
#include <string>
#include <windows.h>
#include "audio.hpp"
#include "application.hpp"
#include "sound_context.hpp"
#include "sound_manager.hpp"
#include "memory_pool.hpp"
#include "context.hpp"
#include "log.hpp"

using namespace types;

triton::cOALAudioBackend::cOALAudioBackend(cContext* context) : iAudioBackend(context)
{
    _alDevice = alcOpenDevice(nullptr);
    _alContext = alcCreateContext(_alDevice, nullptr);
    alcMakeContextCurrent(_alContext);
}

triton::cOALAudioBackend::~cOALAudioBackend()
{
    alcMakeContextCurrent(nullptr);
    alcDestroyContext(_alContext);
    alcCloseDevice(_alDevice);
}

void triton::cOALAudioBackend::CreateSound(sSound& sound)
{
    ALuint source = 0, buffer = 0;

    ALenum wavFormat = AL_FORMAT_STEREO16;
    if (sound.dataFormat == sSound::eDataFormat::STEREO8)
        wavFormat = AL_FORMAT_STEREO8;
    else if (sound.dataFormat == sSound::eDataFormat::MONO16)
        wavFormat = AL_FORMAT_MONO16;
    else if (sound.dataFormat == sSound::eDataFormat::MONO8)
        wavFormat = AL_FORMAT_MONO8;
        
    alGenSources(1, (ALuint*)&source);
    alSourcef(source, AL_PITCH, 1);
    alSourcef(source, AL_GAIN, 1);
    alSource3f(source, AL_POSITION, 0, 0, 0);
    alSource3f(source, AL_VELOCITY, 0, 0, 0);
    alSourcei(source, AL_LOOPING, AL_FALSE);

    alGenBuffers(1, (ALuint*)&buffer);

    alBufferData(buffer, wavFormat, sound.data, sound.dataByteSize, sound.sampleRate);
    alSourcei(source, AL_BUFFER, buffer);

    sound.backendSound = _context->Create<cAudioBackendSound>(_context, source, buffer);
}

void triton::cOALAudioBackend::DestroySound(sSound& sound)
{
    ALuint source = 0, buffer = 0;
    source = sound.backendSound->Source();
    buffer = sound.backendSound->Buffer();

    alDeleteBuffers(1, (ALuint*)&buffer);
    alDeleteSources(1, (ALuint*)&source);
    _context->Destroy<cAudioBackendSound>(sound.backendSound);
}

void triton::cOALAudioBackend::PlaySound(sSound& sound)
{
    alSourcePlay(sound.backendSound->Source());
}

void triton::cOALAudioBackend::StopSound(sSound& sound)
{
    alSourceStop(sound.backendSound->Source());
}

void triton::cOALAudioBackend::SetSoundPosition(sSound& sound, const cVector3& position)
{
    alSource3f(sound.backendSound->Source(), AL_POSITION, position.GetX(), position.GetY(), position.GetZ());
}

void triton::cOALAudioBackend::SetSoundVelocity(sSound& sound, const cVector3& velocity)
{
    alSource3f(sound.backendSound->Source(), AL_VELOCITY, velocity.GetX(), velocity.GetY(), velocity.GetZ());
}

void triton::cOALAudioBackend::SetListenerPosition(const cVector3& position)
{
    alListener3f(AL_POSITION, position.GetX(), position.GetY(), position.GetZ());
}

void triton::cOALAudioBackend::SetListenerVelocity(const cVector3& velocity)
{
    alListener3f(AL_VELOCITY, velocity.GetX(), velocity.GetY(), velocity.GetZ());
}

void triton::cOALAudioBackend::SetListenerOrientation(const cVector3& at, const cVector3& up)
{
    ALfloat values[] = { at.GetX(), at.GetY(), at.GetZ(), up.GetX(), up.GetY(), up.GetZ() };
    alListenerfv(AL_ORIENTATION, &values[0]);
}