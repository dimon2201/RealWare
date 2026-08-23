// image_decoder_multi.hpp

#pragma once

#include <filesystem>
#include <stb_image.h>
#include <tinyddsloader.h>
#include "image_decoder.hpp"

namespace triton
{
	class BImageDecoderMulti : public IImageDecoder
	{
		TRITON_CLASS_NAME(BImageDecoderMulti)

	public:
		explicit BImageDecoderMulti(cContext* context) : IImageDecoder(context) {}
		
		~BImageDecoderMulti() override = default;

		SImageBackend Decode(
			EImageFormat expectedDataFormat,
			const std::filesystem::path& filePath
		) override final;

		SImageBackend Decode(
			EImageFileFormat containerFormat,
			EImageFormat expectedDataFormat,
			const types::u8* data,
			types::usize dataByteSize
		) override final;

	private:
		types::usize DetermineChannelCount(EImageFormat format);

		types::boolean ValidateDDSFormat(
			tinyddsloader::DDSFile::DXGIFormat ddsFormat,
			EImageFormat expectedFormat
		);

		SImageBackend ReturnPNGData(
			EImageFormat expectedDataFormat,
			const std::string& filePathStr,
			const stbi_uc* stbData,
			int stbWidth,
			int stbHeight,
			int stbDesiredChannels
		);

		SImageBackend ReturnDDSData(
			EImageFormat expectedDataFormat,
			const std::string& filePathStr,
			tinyddsloader::Result ddsResult,
			tinyddsloader::DDSFile& dds
		);
	};
}