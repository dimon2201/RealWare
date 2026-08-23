// image_decoder.hpp

#pragma once

#include <filesystem>
#include "backend.hpp"
#include "image_backend_data.hpp"
#include "image_file_format_enum.hpp"
#include "object.hpp"

namespace triton
{
	class IImageDecoder : public iBackend
	{
		TRITON_CLASS_NAME(IImageDecoder)

	public:
		explicit IImageDecoder(cContext* context) : iBackend(context) {}
		
		virtual ~IImageDecoder() = default;

		virtual SImageBackend Decode(
			EImageFormat expectedDataFormat,
			const std::filesystem::path& filePath
		) = 0;

		virtual SImageBackend Decode(
			EImageFileFormat containerFormat,
			EImageFormat expectedDataFormat,
			const types::u8* data,
			types::usize dataByteSize
		) = 0;
	};
}