#include <iostream>
#include <cstdio>
#include <string>
#include <windows.h>

#include "sound_context.hpp"

namespace realware
{
    namespace sound
    {
        sWAVFile* LoadWAVFile(const char* filename);
        void FreeWAVFile(sWAVFile* wavFile);

        cOpenALSoundContext::cOpenALSoundContext()
        {
            m_device = alcOpenDevice(nullptr);
            m_context = alcCreateContext(m_device, nullptr);
            alcMakeContextCurrent(m_context);
        }

        cOpenALSoundContext::~cOpenALSoundContext()
        {
            alcMakeContextCurrent(nullptr);
            alcDestroyContext(m_context);
            alcCloseDevice(m_device);
        }

        sSound* cOpenALSoundContext::CreateSound(const char* filename, const sSound::eFormat& format)
        {
            sSound* sound = nullptr;

            if (format == sSound::eFormat::WAV)
            {
                sound = new sSound();

                sWAVFile* wavFile = LoadWAVFile(filename);
                sound->Format = format;
                sound->File = wavFile;

                alGenSources(1, (ALuint*)&sound->Source);
                alSourcef(sound->Source, AL_PITCH, 1);
                alSourcef(sound->Source, AL_GAIN, 1);
                alSource3f(sound->Source, AL_POSITION, 0, 0, 0);
                alSource3f(sound->Source, AL_VELOCITY, 0, 0, 0);
                alSourcei(sound->Source, AL_LOOPING, AL_FALSE);

                alGenBuffers(1, (ALuint*)&sound->Buffer);

                ALenum wavFormat = AL_FORMAT_STEREO16;
                bool stereo = (wavFile->NumChannels > 1);
                switch (wavFile->BitsPerSample) {
                    case 16:
                        if (stereo) {
                            wavFormat = AL_FORMAT_STEREO16;
                            break;
                        } else {
                            wavFormat = AL_FORMAT_MONO16;
                            break;
                        }
                    case 8:
                        if (stereo) {
                            wavFormat = AL_FORMAT_STEREO8;
                            break;
                        } else {
                            wavFormat = AL_FORMAT_MONO8;
                            break;
                        }
                    default:
                        break;
                }

                alBufferData(sound->Buffer, wavFormat, wavFile->Data, wavFile->DataByteSize, wavFile->SampleRate);
                alSourcei(sound->Source, AL_BUFFER, sound->Buffer);
            }

            return sound;
        }

        void cOpenALSoundContext::FreeSound(const sSound* sound)
        {
            alDeleteSources(1, &sound->Source);
            alDeleteBuffers(1, &sound->Buffer);

            if (sound->Format == sSound::eFormat::WAV)
            {
                free(((sWAVFile*)sound->File)->Data);
                delete (sWAVFile*)sound->File;
            }

            delete sound;
        }

        void cOpenALSoundContext::PlaySound(const sSound* sound)
        {
            alSourcePlay(sound->Source);
        }

        void cOpenALSoundContext::StopSound(const sSound* sound)
        {
            alSourceStop(sound->Source);
        }

        void cOpenALSoundContext::SetSoundPosition(sSound* sound, const glm::vec3& position)
        {
            alSource3f(sound->Source, AL_POSITION, position.x, position.y, position.z);
        }

        void cOpenALSoundContext::SetSoundVelocity(sSound* sound, const glm::vec3& velocity)
        {
            alSource3f(sound->Source, AL_VELOCITY, velocity.x, velocity.y, velocity.z);
        }

        void cOpenALSoundContext::DeleteSound(sSound *sound)
        {
            if (sound->File != nullptr) {
                if (sound->Format == sSound::eFormat::WAV) {
                    FreeWAVFile((sWAVFile*)sound->File);
                }
            }

            alDeleteBuffers(1, (ALuint*)&sound->Buffer);
            alDeleteSources(1, (ALuint*)&sound->Source);
        }

        void cOpenALSoundContext::SetListenerPosition(const glm::vec3& position)
        {
            alListener3f(AL_POSITION, position.x, position.y, position.z);
        }

        void cOpenALSoundContext::SetListenerVelocity(const glm::vec3& velocity)
        {
            alListener3f(AL_VELOCITY, velocity.x, velocity.y, velocity.z);
        }

        void cOpenALSoundContext::SetListenerOrientation(const glm::vec3& at, const glm::vec3& up)
        {
            ALfloat values[] = { at.x, at.y, at.z, up.x, up.y, up.z };
            alListenerfv(AL_ORIENTATION, &values[0]);
        }

        sWAVFile* LoadWAVFile(const char* filename)
        {
            sWAVFile* wav = new sWAVFile();

            FILE* fp = nullptr;
            errno_t err = fopen_s(&fp, &filename[0], "rb");
            if (err != 0)
            {
                std::cout << "Error opening WAV file at '" << filename << "'!" << std::endl;
                MessageBox(0, "Error opening WAV file", 0, MB_ICONERROR);
            }

            // Chunk
            fread(&wav->Type[0], sizeof(char), 4, fp);
            if (std::string(wav->Type) != std::string("RIFF")) {
                std::cout << "Not a RIFF file!" << std::endl;
            }

            fread(&wav->ChunkSize, sizeof(int), 1, fp);
            fread(&wav->Format[0], sizeof(char), 4, fp);
            if (std::string(wav->Format) != std::string("WAVE")) {
                std::cout << "Not a WAVE file!" << std::endl;
            }

            // 1st Subchunk
            fread(&wav->Subchunk1ID[0], sizeof(char), 4, fp);
            if (std::string(wav->Subchunk1ID) != std::string("fmt ")) {
                std::cout << "Missing fmt header!" << std::endl;
            }
            fread(&wav->Subchunk1Size, sizeof(int), 1, fp);
            fread(&wav->AudioFormat, sizeof(short), 1, fp);
            fread(&wav->NumChannels, sizeof(short), 1, fp);
            fread(&wav->SampleRate, sizeof(int), 1, fp);
            fread(&wav->ByteRate, sizeof(int), 1, fp);
            fread(&wav->BlockAlign, sizeof(short), 1, fp);
            fread(&wav->BitsPerSample, sizeof(short), 1, fp);

            // 2nd Subchunk
            fread(&wav->Subchunk2ID[0], sizeof(char), 4, fp);
            if (std::string(wav->Subchunk2ID) != std::string("data")) {
                std::cout << "Missing data header!" << std::endl;
            }
            fread(&wav->Subchunk2Size, sizeof(int), 1, fp);

            // Data
            int NumSamples = wav->Subchunk2Size / (wav->NumChannels * (wav->BitsPerSample / 8));
            wav->DataByteSize = NumSamples * (wav->BitsPerSample / 8) * wav->NumChannels;
            wav->Data = (unsigned short*)malloc(wav->DataByteSize);
            if (wav->BitsPerSample == 16 && wav->NumChannels == 2)
            {
                for (int i = 0; i < NumSamples; i++)
                {
                    int idx = i * 2;
                    fread(&wav->Data[idx], sizeof(short), 1, fp);
                    fread(&wav->Data[idx + 1], sizeof(short), 1, fp);
                }
            }
            fclose(fp);

            return wav;
        }

        void FreeWAVFile(sWAVFile* wavFile)
        {
            if (wavFile != nullptr)
            {
                free(wavFile->Data);
                delete wavFile;
            }
        }
    }
}