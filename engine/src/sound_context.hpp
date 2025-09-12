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
            types::u8 Type[5];
            types::u8 Format[5];
            types::u8 Subchunk1ID[5];
            types::u8 Subchunk2ID[5];
            types::u32 ChunkSize;
            types::u32 Subchunk1Size;
            types::u32 SampleRate;
            types::u32 ByteRate;
            types::u32 Subchunk2Size;
            types::u16 AudioFormat;
            types::u16 NumChannels;
            types::u16 BlockAlign;
            types::u16 BitsPerSample;
            types::u32 NumSamples;
            types::u32 DataByteSize;
            types::u16* Data;
        };

        struct sSound
        {
            enum class eFormat
            {
                NONE = 0,
                WAV = 1
            };

            std::string Tag = "";
            eFormat Format = eFormat::NONE;
            void* File = nullptr;
            types::u32 Source;
            types::u32 Buffer;
        };

        class cSoundContext
        {
        public:
            cSoundContext() = default;
            virtual ~cSoundContext() = default;

            virtual sSound* Create(const std::string& filename, const sSound::eFormat& format) = 0;
            virtual void Destroy(const sSound* const sound) = 0;
            virtual void Play(const sSound* const sound) = 0;
            virtual void Stop(const sSound* const sound) = 0;
            virtual void SetPosition(const sSound* const sound, const glm::vec3& position) = 0;
            virtual void SetVelocity(const sSound* const sound, const glm::vec3& velocity) = 0;
            virtual void SetListenerPosition(const glm::vec3& position) = 0;
            virtual void SetListenerVelocity(const glm::vec3& velocity) = 0;
            virtual void SetListenerOrientation(const glm::vec3& at, const glm::vec3& up) = 0;

        };

        class cOpenALSoundContext : public cSoundContext
        {
        public:
            cOpenALSoundContext();
            virtual ~cOpenALSoundContext() override final;

            virtual sSound* Create(const std::string& filename, const sSound::eFormat& format) override final;
            virtual void Destroy(const sSound* const sound) override final;
            virtual void Play(const sSound* const sound) override final;
            virtual void Stop(const sSound* const sound) override final;
            virtual void SetPosition(const sSound* const sound, const glm::vec3& position) override final;
            virtual void SetVelocity(const sSound* const sound, const glm::vec3& velocity) override final;
            virtual void SetListenerPosition(const glm::vec3& position) override final;
            virtual void SetListenerVelocity(const glm::vec3& velocity) override final;
            virtual void SetListenerOrientation(const glm::vec3& at, const glm::vec3& up) override final;

        private:
            ALCdevice* m_device = nullptr;
            ALCcontext* m_context = nullptr;
        };
    }
}