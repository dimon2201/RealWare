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
            core::u8 Type[5];
            core::u8 Format[5];
            core::u8 Subchunk1ID[5];
            core::u8 Subchunk2ID[5];
            core::u32 ChunkSize;
            core::u32 Subchunk1Size;
            core::u32 SampleRate;
            core::u32 ByteRate;
            core::u32 Subchunk2Size;
            core::u16 AudioFormat;
            core::u16 NumChannels;
            core::u16 BlockAlign;
            core::u16 BitsPerSample;
            core::u32 NumSamples;
            core::u32 DataByteSize;
            core::u16* Data;
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
            core::u32 Source;
            core::u32 Buffer;
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