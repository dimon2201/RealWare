// TRF.hpp

#pragma once

#include <optional>
#include <filesystem>
#include "types.hpp"

namespace triton
{
	namespace resource_file
	{
		enum class EResourceFormat
		{
			Model3D
		};

		enum class ETextureFormat
		{
			PNG,
			DDS
		};

		enum class ETextureDataFormat
		{
			R8,
			RGB8,
			RGBA8,
			RGBA8_SRGB
		};

		struct SModel3DData
		{
		};

		struct STextureData
		{
			ETextureFormat format;
			ETextureDataFormat dataFormat;
			types::usize width = 0;
			types::usize height = 0;
			types::usize channelCount = 0;
			types::u8* pixelByteData = nullptr;
		};

		struct SModel3DMaterialData
		{
			std::string diffuseTexturePath = "";
			std::string normalTexturePath = "";
			std::string roughnessTexturePath = "";
			std::string metallicTexturePath = "";
			types::boolean bIsDiffuseEmbedded = types::K_FALSE;
			types::boolean bIsNormalEmbedded = types::K_FALSE;
			types::boolean bIsRoughnessEmbedded = types::K_FALSE;
			types::boolean bIsMetallicEmbedded = types::K_FALSE;
			STextureData diffuseTexture;
			STextureData normalTexture;
			STextureData roughnessTexture;
			STextureData metallicTexture;
		};

		template <EResourceFormat TResourceFormat>
		class CResourceFile
		{
			types::usize _dataByteSize = 0;
			types::u8* _data = nullptr;
			std::string _dataFolderPath = "";
			types::usize _parsedDataByteSize = 0;
			types::u8* _parsedData = nullptr;

		public:
			CResourceFile() = delete;
			explicit CResourceFile(const std::filesystem::path& filePath);
			~CResourceFile();

			void Parse();

		private:
			std::optional<SModel3DData> ParseModel3D();

			std::optional<STextureData> ParseModel3DTexture(
				types::boolean bIsEmbeddedTexture,
				const aiTexture* assimpTexture,
				const std::filesystem::path& modelFolderPath,
				const std::filesystem::path& textureLocalFilePath
			);

			types::boolean ParsePNG(
				types::usize width,
				types::usize height,
				types::usize channels,
				types::u8* pixels,
				ETextureFormat& outFormat,
				ETextureDataFormat& outDataFormat,
				types::usize& outWidth,
				types::usize& outHeight,
				types::usize& outChannels,
				types::u8*& outPixels
			);

			types::boolean CResourceFile::ParseDDS(
				types::usize width,
				types::usize height,
				tinyddsloader::DDSFile::DXGIFormat format,
				types::u8* pixels,
				ETextureFormat& outFormat,
				ETextureDataFormat& outDataFormat,
				types::usize& outWidth,
				types::usize& outHeight,
				types::usize& outChannels,
				types::u8*& outPixels
			);
		};
	}
}