#pragma once

#include <vector>
#include <AL/al.h>
#include <AL/alc.h>
#include "../../thirdparty/glm/glm/glm.hpp"
#include "types.hpp"

namespace realware
{
    namespace sound
    {
        struct sWAVFile
        {
            char Type[5];
            char Format[5];
            char Subchunk1ID[5];
            char Subchunk2ID[5];
            int ChunkSize;
            int Subchunk1Size;
            int SampleRate;
            int ByteRate;
            int Subchunk2Size;
            short AudioFormat;
            short NumChannels;
            short BlockAlign;
            short BitsPerSample;
            unsigned NumSamples;
            unsigned DataByteSize;
            unsigned short* Data;
        };

        struct sSound
        {
            enum eFormat
            {
                WAV = 0
            };

            std::string Tag = "";
            eFormat Format;
            void* File;
            core::u32 Source;
            core::u32 Buffer;
        };

        class cSoundContext
        {

        public:
            cSoundContext() {}
            ~cSoundContext() {}

            virtual sSound* CreateSound(const char* filename, const sSound::eFormat& format) = 0;
            virtual void FreeSound(const sSound* sound) = 0;
            virtual void PlaySound(const sSound* sound) = 0;
            virtual void StopSound(const sSound* sound) = 0;
            virtual void SetSoundPosition(sSound* sound, const glm::vec3& position) = 0;
            virtual void SetSoundVelocity(sSound* sound, const glm::vec3& velocity) = 0;
            virtual void DeleteSound(sSound* sound) = 0;
            virtual void SetListenerPosition(const glm::vec3& position) = 0;
            virtual void SetListenerVelocity(const glm::vec3& velocity) = 0;
            virtual void SetListenerOrientation(const glm::vec3& at, const glm::vec3& up) = 0;

        };

        class cOpenALSoundContext : public cSoundContext
        {

        public:
            cOpenALSoundContext();
            ~cOpenALSoundContext();

            virtual sSound* CreateSound(const char* filename, const sSound::eFormat& format) override final;
            virtual void FreeSound(const sSound* sound) override final;
            virtual void PlaySound(const sSound* sound) override final;
            virtual void StopSound(const sSound* sound) override final;
            virtual void SetSoundPosition(sSound* sound, const glm::vec3& position) override final;
            virtual void SetSoundVelocity(sSound* sound, const glm::vec3& velocity) override final;
            virtual void DeleteSound(sSound* sound) override final;
            virtual void SetListenerPosition(const glm::vec3& position) override final;
            virtual void SetListenerVelocity(const glm::vec3& velocity) override final;
            virtual void SetListenerOrientation(const glm::vec3& at, const glm::vec3& up) override final;

        private:
            ALCdevice* m_device;
            ALCcontext* m_context;

        };
    }
}