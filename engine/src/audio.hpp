// audio.hpp

#pragma once

#include "subsystem.hpp"
#include "types.hpp"

namespace triton
{
	class cAudioBackendSound;
	class iAudioBackend;
	class XDataBuffer;

	class cAudio : public CSubsystem
	{
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
		cAudio() = default;
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