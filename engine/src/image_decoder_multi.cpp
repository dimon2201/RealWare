// image_decoder_multi.hpp

#include <cctype>
#include <cstring>
#include "image_decoder_multi.hpp"
#include "object_allocator.hpp"
#include "log.hpp"

using namespace types;

triton::SImageBackend triton::BImageDecoderMulti::Decode(
	EImageFormat expectedDataFormat,
	const std::filesystem::path& filePath
)
{
	const std::string filePathStr = filePath.string();

	std::string ext = filePath.extension().string();
	for (usize i = 0; i < ext.size(); i++)
		ext[i] = std::tolower((types::u8)ext[i]);

	if (ext == ".png")
	{
		int stbWidth = 0;
		int stbHeight = 0;
		int stbChannels = 0;
		int stbDesiredChannels = (int)DetermineChannelCount(expectedDataFormat);

		stbi_uc* stbData = stbi_load(
			filePathStr.c_str(),
			&stbWidth,
			&stbHeight,
			&stbChannels,
			stbDesiredChannels
		);

		return ReturnPNGData(
			expectedDataFormat,
			filePathStr,
			stbData,
			stbWidth,
			stbHeight,
			stbDesiredChannels
		);
	}
	else if (ext == ".dds")
	{
		tinyddsloader::DDSFile dds;
		tinyddsloader::Result ddsResult = dds.Load(filePathStr.c_str());

		return ReturnDDSData(
			expectedDataFormat,
			filePathStr,
			ddsResult,
			dds
		);
	}
	else
	{
		Print("Error: unsupported image format: " + filePathStr);

		return {};
	}
}

triton::SImageBackend triton::BImageDecoderMulti::Decode(
	EImageFileFormat containerFormat,
	EImageFormat expectedDataFormat,
	const types::u8* data,
	types::usize dataByteSize
)
{
	const std::string filePathStr = "";

	if (containerFormat == EImageFileFormat::PNG)
	{
		int stbWidth = 0;
		int stbHeight = 0;
		int stbChannels = 0;
		int stbDesiredChannels = (int)DetermineChannelCount(expectedDataFormat);

		stbi_uc* stbData = stbi_load_from_memory(
			(const stbi_uc*)data,
			(int)dataByteSize,
			&stbWidth,
			&stbHeight,
			&stbChannels,
			stbDesiredChannels
		);

		return ReturnPNGData(
			expectedDataFormat,
			filePathStr,
			stbData,
			stbWidth,
			stbHeight,
			stbDesiredChannels
		);
	}
	else if (containerFormat == EImageFileFormat::DDS)
	{
		tinyddsloader::DDSFile dds;
		tinyddsloader::Result ddsResult = dds.Load(
			(const uint8_t*)data,
			(size_t)dataByteSize
		);

		return ReturnDDSData(
			expectedDataFormat,
			filePathStr,
			ddsResult,
			dds
		);
	}
	else
	{
		Print("Error: unsupported byte image format");

		return {};
	}
}

triton::EImageFormat triton::BImageDecoderMulti::TextureFormatToImageFormat(ETextureFormat textureFormat)
{
	if (textureFormat == ETextureFormat::R8)
		return EImageFormat::R8;
	else if (textureFormat == ETextureFormat::RGBA8)
		return EImageFormat::RGBA8;
	else if (textureFormat == ETextureFormat::RGBA8_SRGB_Mips)
		return EImageFormat::RGBA8_SRGB;

	return EImageFormat::Unknown;
}

usize triton::BImageDecoderMulti::DetermineChannelCount(EImageFormat format)
{
	if (format == EImageFormat::R8)
		return 1;
	else if (format == EImageFormat::RGBA8)
		return 4;
	else if (format == EImageFormat::RGBA8_SRGB)
		return 4;

	return 0;
}

boolean triton::BImageDecoderMulti::ValidateDDSFormat(
	tinyddsloader::DDSFile::DXGIFormat ddsFormat,
	EImageFormat expectedFormat
)
{
	if (ddsFormat == tinyddsloader::DDSFile::DXGIFormat::R8_UNorm &&
		expectedFormat == EImageFormat::R8)
		return True;
	else if (ddsFormat == tinyddsloader::DDSFile::DXGIFormat::R8G8B8A8_UNorm &&
		expectedFormat == EImageFormat::RGBA8)
		return True;
	else if (ddsFormat == tinyddsloader::DDSFile::DXGIFormat::R8G8B8A8_UNorm_SRGB &&
		expectedFormat == EImageFormat::RGBA8_SRGB)
		return True;

	return False;
}

triton::SImageBackend triton::BImageDecoderMulti::ReturnPNGData(
	EImageFormat expectedDataFormat,
	const std::string& filePathStr,
	const stbi_uc* stbData,
	int stbWidth,
	int stbHeight,
	int stbDesiredChannels
)
{
	if (!stbData)
	{
		const char* reason = stbi_failure_reason();

		if (filePathStr != "")
		{
			Print(
				"Error: failed to decode image: " +
				filePathStr +
				", reason: " +
				(reason ? reason : "unknown")
			);
		}
		else
		{
			Print(
				"Error: failed to decode byte image" +
				std::string(", reason: ") +
				(reason ? reason : "unknown")
			);
		}

		return {};
	}

	const usize imageDataByteSize = (usize)(stbWidth * stbHeight * stbDesiredChannels);
	const cVector3 imageSize = cVector3((f32)stbWidth, (f32)stbHeight, 0.0f);

	u8* imageData = (u8*)CObjectAllocator::Allocate(imageDataByteSize, 64);
	if (!imageData)
	{
		stbi_image_free((void*)stbData);
		return {};
	}
	std::memcpy(&imageData[0], &stbData[0], imageDataByteSize);
	stbi_image_free((void*)stbData);

	SImageBackend image;
	image.data = imageData;
	image.dataByteSize = imageDataByteSize;
	image.dataFormat = expectedDataFormat;
	image.size = imageSize;

	return image;
}

triton::SImageBackend triton::BImageDecoderMulti::ReturnDDSData(
	EImageFormat expectedDataFormat,
	const std::string& filePathStr,
	tinyddsloader::Result ddsResult,
	tinyddsloader::DDSFile& dds
)
{
	if (ddsResult != tinyddsloader::Success)
	{
		if (filePathStr != "")
		{
			Print(
				"Error: failed to decode image: " +
				filePathStr +
				", result: " +
				std::to_string((int)ddsResult)
			);
		}
		else
		{
			Print(
				"Error: failed to decode byte image" +
				std::string(", result: ") +
				std::to_string((int)ddsResult)
			);
		}

		return {};
	}

	tinyddsloader::DDSFile::DXGIFormat ddsFormat = dds.GetFormat();
	boolean validationResult = ValidateDDSFormat(ddsFormat, expectedDataFormat);
	if (validationResult == False)
	{
		std::string error =
			"Error: DDS image format does not match expected format";

		if (filePathStr != "")
			error += ": " + filePathStr;

		Print(error);

		return {};
	}

	uint32_t ddsWidth = dds.GetWidth();
	uint32_t ddsHeight = dds.GetHeight();
	void* ddsData = dds.GetImageData()->m_mem;

	const usize channelCount = DetermineChannelCount(expectedDataFormat);

	const usize imageDataByteSize = (usize)(ddsWidth * ddsHeight) * channelCount;
	const cVector3 imageSize = cVector3((f32)ddsWidth, (f32)ddsHeight, 0.0f);

	u8* imageData = (u8*)CObjectAllocator::Allocate(imageDataByteSize, 64);
	if (!imageData)
		return {};
	std::memcpy(&imageData[0], ddsData, imageDataByteSize);

	SImageBackend image;
	image.data = imageData;
	image.dataByteSize = imageDataByteSize;
	image.dataFormat = expectedDataFormat;
	image.size = imageSize;

	return image;
}