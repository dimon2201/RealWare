// texture.cpp

#include "texture.hpp"
#include "context.hpp"
#include "synchronization.hpp"

using namespace types;

triton::XTexture::XTexture(
	cContext* context,
	s32 poolIndex,
	ETextureFormat expectedDataFormat,
	const std::filesystem::path& filePath
) : iObject(context, poolIndex)
{
}

triton::XTexture::XTexture(
	cContext* context,
	s32 poolIndex,
	ETextureFileFormat containerFormat,
	ETextureFormat expectedDataFormat,
	const u8* byteData,
	usize byteDataByteSize,
	const cVector2& textureSize
) : iObject(context, poolIndex)
{
}

triton::XTexture::~XTexture()
{
}