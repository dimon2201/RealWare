// audio.cpp

#include <fstream>
#include "application.hpp"
#include "audio.hpp"
#include "context.hpp"
#include "sound_context.hpp"
#include "engine.hpp"
#include "components.hpp"
#include "log.hpp"

using namespace types;

using namespace triton::ecs;
using namespace triton::ecs::components;

triton::cAudio::cAudio(cContext* context, API api) : cSystem(context), _backendAPI(api)
{
	if (api == API::NONE)
	{
		Print("Error: sound API not selected!");

		return;
	}
	else if (api == API::OAL)
	{
		_backend = new cOALAudioBackend(_context);
	}
}

triton::cAudio::~cAudio()
{
	if (_backendAPI == API::NONE)
	{
		Print("Error: sound API not selected!");

		return;
	}
	else if (_backendAPI == API::OAL)
	{
		if (_backend)
			delete _backend;
	}
}

void triton::cAudio::OnFrameUpdate() {}

void triton::cAudio::CreateSound(sSound::eContainerFormat format, const std::string& filePath)
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

	sSound sound = {};

	const sCapabilities* caps = _context->GetSubsystem<cEngine>()->GetApplication()->GetCapabilities();
	cMemoryAllocator* memoryAllocator = _context->GetMemoryAllocator();

	if (format == sSound::eContainerFormat::WAV)
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
		sound.data = (u16*)memoryAllocator->Allocate(sound.dataByteSize, caps->memoryAlignment);
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
		sSound::eDataFormat dataFormat = sSound::eDataFormat::NONE;
		types::boolean stereo = sound.channelCount > 1;
		switch (sound.bitsPerSample)
		{
			case 16:
				if (stereo)
				{
					sound.dataFormat = sSound::eDataFormat::STEREO16;
					break;
				}
				else
				{
					sound.dataFormat = sSound::eDataFormat::MONO16;
					break;
				}
			case 8:
				if (stereo)
				{
					sound.dataFormat = sSound::eDataFormat::STEREO8;
					break;
				}
				else
				{
					sound.dataFormat = sSound::eDataFormat::MONO8;
					break;
				}
			default:
				break;
		}
	}

	_backend->CreateSound(sound);
}