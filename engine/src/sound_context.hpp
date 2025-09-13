#pragma once

#include <vector>
#include <AL/al.h>
#include <AL/alc.h>
#include "../../thirdparty/glm/glm/glm.hpp"
#include "category.hpp"
#include "id_vec.hpp"
#include "types.hpp"

namespace realware
{
    namespace sound
    {
        struct sWAVStructure;
        class cSound;

        class cSoundContext
        {
        public:
            cSoundContext() = default;
            virtual ~cSoundContext() = default;

            virtual void Create(const std::string& filename, const game::Category& format, const sWAVStructure** const file, types::u32& source, types::u32& buffer) = 0;
            virtual void Destroy(const cSound* const sound) = 0;
            virtual void Play(const cSound* const sound) = 0;
            virtual void Stop(const cSound* const sound) = 0;
            virtual void SetPosition(const cSound* const sound, const glm::vec3& position) = 0;
            virtual void SetVelocity(const cSound* const sound, const glm::vec3& velocity) = 0;
            virtual void SetListenerPosition(const glm::vec3& position) = 0;
            virtual void SetListenerVelocity(const glm::vec3& velocity) = 0;
            virtual void SetListenerOrientation(const glm::vec3& at, const glm::vec3& up) = 0;
        };

        class cOpenALSoundContext : public cSoundContext
        {
        public:
            cOpenALSoundContext();
            virtual ~cOpenALSoundContext() override final;

            virtual void Create(const std::string& filename, const game::Category& format, const sWAVStructure** const file, types::u32& source, types::u32& buffer) override final;
            virtual void Destroy(const cSound* const sound) override final;
            virtual void Play(const cSound* const sound) override final;
            virtual void Stop(const cSound* const sound) override final;
            virtual void SetPosition(const cSound* const sound, const glm::vec3& position) override final;
            virtual void SetVelocity(const cSound* const sound, const glm::vec3& velocity) override final;
            virtual void SetListenerPosition(const glm::vec3& position) override final;
            virtual void SetListenerVelocity(const glm::vec3& velocity) override final;
            virtual void SetListenerOrientation(const glm::vec3& at, const glm::vec3& up) override final;

        private:
            ALCdevice* m_device = nullptr;
            ALCcontext* m_context = nullptr;
        };
    }
}