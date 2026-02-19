// audio_backend_oal.cpp

#include "audio_backend_oal.hpp"

using namespace types;

triton::cAudioBackendOAL::cAudioBackendOAL(cContext* context) : iAudioBackend(context)
{
    _alDevice = alcOpenDevice(nullptr);
    _alContext = alcCreateContext(_alDevice, nullptr);
    alcMakeContextCurrent(_alContext);
}

triton::cAudioBackendOAL::~cAudioBackendOAL()
{
    alcMakeContextCurrent(nullptr);
    alcDestroyContext(_alContext);
    alcCloseDevice(_alDevice);
}

triton::sAudioBackendSound triton::cAudioBackendOAL::CreateSound(
    cAudio::eDataFormat dataFormat,
    const u8* data,
    usize dataByteSize,
    usize sampleRate
)
{
    sAudioBackendSound abs = {};
    
    ALenum wavFormat = AL_FORMAT_STEREO16;
    if (dataFormat == cAudio::eDataFormat::STEREO8)
        wavFormat = AL_FORMAT_STEREO8;
    else if (dataFormat == cAudio::eDataFormat::MONO16)
        wavFormat = AL_FORMAT_MONO16;
    else if (dataFormat == cAudio::eDataFormat::MONO8)
        wavFormat = AL_FORMAT_MONO8;
    
    alGenSources(1, (ALuint*)&abs.source);
    alSourcef(abs.source, AL_PITCH, 1);
    alSourcef(abs.source, AL_GAIN, 1);
    alSource3f(abs.source, AL_POSITION, 0, 0, 0);
    alSource3f(abs.source, AL_VELOCITY, 0, 0, 0);
    alSourcei(abs.source, AL_LOOPING, AL_FALSE);

    alGenBuffers(1, (ALuint*)&abs.buffer);

    alBufferData(abs.buffer, wavFormat, data, dataByteSize, sampleRate);
    alSourcei(abs.source, AL_BUFFER, abs.buffer);

    return abs;
}

void triton::cAudioBackendOAL::DestroySound(sAudioBackendSound& sound)
{
    alDeleteBuffers(1, (ALuint*)&sound.buffer);
    alDeleteSources(1, (ALuint*)&sound.source);
}

void triton::cAudioBackendOAL::PlaySound(sAudioBackendSound& sound)
{
    alSourcePlay(sound.source);
}

void triton::cAudioBackendOAL::StopSound(sAudioBackendSound& sound)
{
    alSourceStop(sound.source);
}

void triton::cAudioBackendOAL::SetSoundPosition(sAudioBackendSound& sound, const cVector3& position)
{
    alSource3f(sound.source, AL_POSITION, position.GetX(), position.GetY(), position.GetZ());
}

void triton::cAudioBackendOAL::SetSoundVelocity(sAudioBackendSound& sound, const cVector3& velocity)
{
    alSource3f(sound.source, AL_VELOCITY, velocity.GetX(), velocity.GetY(), velocity.GetZ());
}

void triton::cAudioBackendOAL::SetListenerPosition(const cVector3& position)
{
    alListener3f(AL_POSITION, position.GetX(), position.GetY(), position.GetZ());
}

void triton::cAudioBackendOAL::SetListenerVelocity(const cVector3& velocity)
{
    alListener3f(AL_VELOCITY, velocity.GetX(), velocity.GetY(), velocity.GetZ());
}

void triton::cAudioBackendOAL::SetListenerOrientation(const cVector3& at, const cVector3& up)
{
    ALfloat values[] = { at.GetX(), at.GetY(), at.GetZ(), up.GetX(), up.GetY(), up.GetZ() };
    alListenerfv(AL_ORIENTATION, &values[0]);
}