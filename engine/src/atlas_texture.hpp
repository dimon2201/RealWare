// atlas_texture.hpp

#pragma once

#include <filesystem>
#include "object.hpp"
#include "math.hpp"
#include "handle.hpp"
#include "texture_formats.hpp"
#include "texture_file_formats.hpp"
#include "types.hpp"

namespace triton
{
	struct STextureAtlasRegion
	{
		cVector2 offsetNorm = cVector2(0.0f);
		cVector2 sizeNorm = cVector2(0.0f);
		cVector2 offsetPixel = cVector2(0.0f);
		cVector2 sizePixel = cVector2(0.0f);
		types::u32 zAtlasLayer = 0;
	};

	class XAtlasTexture : public iObject
	{
		TRITON_OBJECT(XAtlasTexture)

		STextureAtlasRegion _region;

	public:
		explicit XAtlasTexture(
			cContext* context,
			types::s32 poolIndex,
			const std::filesystem::path& filePath,
			ETextureFormat dataFormat
		);

		explicit XAtlasTexture(
			cContext* context,
			types::s32 poolIndex,
			const types::u8* byteData,
			types::usize byteDataByteSize,
			types::usize width,
			types::usize height,
			types::usize channelCount,
			ETextureFileFormat fileFormat,
			ETextureFormat byteDataFormat
		);

		~XAtlasTexture() override;

		inline cVector2 GetOffsetNorm() const { return _region.offsetNorm; }

		inline cVector2 GetSizeNorm() const { return _region.sizeNorm; }

		inline types::u32 GetAtlasZLayer() const { return _region.zAtlasLayer; }

		struct THandle : public SHandle {};

		struct TGPULayout {};
	};
}