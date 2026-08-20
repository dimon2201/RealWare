// atlas_texture.cpp

#include "atlas_texture.hpp"
#include "texture_atlas.hpp"
#include "context.hpp"

triton::XAtlasTexture::XAtlasTexture(
	cContext* context,
	types::s32 poolIndex,
	const std::filesystem::path& filePath,
	ETextureFormat dataFormat
) : iObject(context, poolIndex)
{
	auto result = _context->GetSubsystem<CTextureAtlas>()->Create(filePath, dataFormat);
	if (result.has_value())
		_region = *result;
	else
		Print("Error: can't create atlas texture!");
}

triton::XAtlasTexture::XAtlasTexture(
	cContext* context,
	types::s32 poolIndex,
	const types::u8* byteData,
	types::usize byteDataByteSize,
	types::usize width,
	types::usize height,
	types::usize channelCount,
	ETextureFileFormat fileFormat,
	ETextureFormat byteDataFormat
) : iObject(context, poolIndex)
{
	auto result = _context->GetSubsystem<CTextureAtlas>()->Create(
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