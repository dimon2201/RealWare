// audio.cpp

#include <fstream>
#include "application.hpp"
#include "audio.hpp"
#include "context.hpp"
#include "engine.hpp"
#include "log.hpp"

using namespace types;

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

triton::cAudio::cAudio(cContext* context) : CSubsystem(context) {}

void triton::cAudio::OnFrameUpdate() {}

void triton::cAudio::CreateSound(eContainerFormat format, const std::string& filePath)
{
	// TODO: creation of sound
	// SoundComponent& soundComp = scenes.GetComponent<SoundComponent>(entity);
	// soundComp.sound = audioSystem.CreateSound(soundFormat, soundFilePath);
	// soundComp.playState = cAudio::ePlayState::PLAY;

	std::ifstream inputFile(filePath, std::ios::binary);
	if (!inputFile)
	{
		Print("Error: can't open sound file at '" + filePath + "'!");

		return;
	}

	sAudioSound sound = {};

	const sCapabilities& caps = _context->GetSubsystem<CEngine>()->GetApplication()->GetCapabilities();
	cMemoryAllocator* memoryAllocator = _context->GetMemoryAllocator();

	if (format == eContainerFormat::WAV)
	{
		sWAVHeader wh = {};

		// Chunk
		inputFile.read((char*)&wh._type[0], 4);
		if (std::string((const char*)&wh._type[0]) != std::string("RIFF"))
			Print("Error: not a RIFF file!");
		inputFile.read((char*)&wh._chunkSize, sizeof(types::u32));
		inputFile.read((char*)&wh._format[0], 4);
		if (std::string((const char*)&wh._format[0]) != std::string("WAVE"))
			Print("Error: not a WAVE file!");

		// 1st Subchunk
		inputFile.read((char*)&wh._subchunk1ID, 4);
		if (std::string((const char*)&wh._subchunk1ID[0]) != std::string("fmt "))
			Print("Error: missing fmt header!");
		inputFile.read((char*)&wh._subchunk1Size, sizeof(types::u32));
		inputFile.read((char*)&wh._audioFormat, sizeof(types::u16));
		inputFile.read((char*)&sound.channelCount, sizeof(types::u16));
		inputFile.read((char*)&sound.sampleRate, sizeof(types::u32));
		inputFile.read((char*)&wh._byteRate, sizeof(types::u32));
		inputFile.read((char*)&wh._blockAlign, sizeof(types::u16));
		inputFile.read((char*)&sound.bitsPerSample, sizeof(types::u16));

		// 2nd Subchunk
		inputFile.read((char*)&wh._subchunk2ID, 4);
		if (std::string((const char*)&wh._subchunk2ID[0]) != std::string("data"))
			Print("Error: missing data header!");
		inputFile.read((char*)&wh._subchunk2Size, sizeof(types::u32));

		// Data
		const usize numSamples = wh._subchunk2Size / (sound.channelCount * (sound.bitsPerSample / 8));
		sound.dataByteSize = numSamples * (sound.bitsPerSample / 8) * sound.channelCount;
		sound.data = (u16*)memoryAllocator->Allocate(sound.dataByteSize, caps.memoryAlignment);
		if (sound.bitsPerSample == 16 && sound.channelCount == 2)
		{
			for (usize i = 0; i < numSamples; i++)
			{
				const usize idx = i * 2;
				inputFile.read((char*)&sound.data[idx], sizeof(u16));
				inputFile.read((char*)&sound.data[idx + 1], sizeof(u16));
			}
		}
		inputFile.close();
		
		// Choose data format
		eDataFormat dataFormat = eDataFormat::NONE;
		types::boolean stereo = sound.channelCount > 1;
		switch (sound.bitsPerSample)
		{
			case 16:
				if (stereo)
				{
					sound.dataFormat = eDataFormat::STEREO16;
					break;
				}
				else
				{
					sound.dataFormat = eDataFormat::MONO16;
					break;
				}
			case 8:
				if (stereo)
				{
					sound.dataFormat = eDataFormat::STEREO8;
					break;
				}
				else
				{
					sound.dataFormat = eDataFormat::MONO8;
					break;
				}
			default:
				break;
		}
	}
}