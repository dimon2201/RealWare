// audio.hpp

#pragma once

#include "category.hpp"
#include "ecs.hpp"
#include "system.hpp"
#include "types.hpp"

namespace triton
{
	class cAudioBackendSound;
	class iAudioBackend;
	class cDataBuffer;

	struct sWAVHeader
	{
		types::u8 _type[5] = {};
		types::u8 _format[5] = {};
		types::u8 _subchunk1ID[5] = {};
		types::u8 _subchunk2ID[5] = {};
		types::u32 _chunkSize = 0;
		types::u32 _subchunk1Size = 0;
		types::u32 _sampleRate = 0;
		types::u32 _byteRate = 0;
		types::u32 _subchunk2Size = 0;
		types::u16 _audioFormat = 0;
		types::u16 _numChannels = 0;
		types::u16 _blockAlign = 0;
		types::u16 _bitsPerSample = 0;
		types::u32 _numSamples = 0;
	};

	class cAudio : public ecs::cSystem
	{
		TRITON_OBJECT(cAudio)

	public:
		enum class eContainerFormat
		{
			NONE = 0,
			WAV
		};

		enum class eDataFormat
		{
			NONE = 0,
			STEREO16,
			MONO16,
			STEREO8,
			MONO8
		};

		explicit cAudio(cContext* context);
		virtual ~cAudio() override final = default;
		virtual void OnFrameUpdate();
		void CreateSound(eContainerFormat format, const ::std::string& filePath);
	};

	struct sAudioSound
	{
		cAudioBackendSound* backendSound = nullptr;
		cAudio::eContainerFormat containerFormat = cAudio::eContainerFormat::NONE;
		cAudio::eDataFormat dataFormat = cAudio::eDataFormat::NONE;
		types::u16 channelCount = 0;
		types::u16 bitsPerSample = 0;
		types::u32 sampleRate = 0;
		types::u16* data = nullptr;
		types::usize dataByteSize = 0;
	};
}