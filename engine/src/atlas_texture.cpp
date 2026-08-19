// atlas_texture.cpp

#include "atlas_texture.hpp"
#include "texture_atlas.hpp"

triton::XAtlasTexture::XAtlasTexture(
	cContext* context,
	const std::filesystem::path& filePath,
	ETextureFormat dataFormat
)
{
	auto result = _context->GetSubsystem<XTextureAtlas>()->Create(filePath, dataFormat);
	if (result.has_value())
		_region = *result;
	else
		Print("Error: can't create atlas texture!");
}

triton::XAtlasTexture::XAtlasTexture(
	cContext* context,
	const types::u8* byteData,
	types::usize byteDataByteSize,
	types::usize width,
	types::usize height,
	types::usize channelCount,
	ETextureFileFormat fileFormat,
	ETextureFormat byteDataFormat
)
{
	auto result = _context->GetSubsystem<XTextureAtlas>()->Create(
		byteData,
		byteDataByteSize,
		width,
		height,
		channelCount,
		fileFormat,
		byteDataFormat
	);
	if (result.has_value())
		_region = *result;
	else
		Print("Error: can't create atlas texture!");
}

triton::XAtlasTexture::~XAtlasTexture() {}