// texture.cpp

#include "texture.hpp"
#include "context.hpp"
#include "synchronization.hpp"
#include "image_decoder.hpp"

using namespace types;

triton::XTexture::XTexture(
	cContext* context,
	s32 poolIndex,
	ETextureFormat textureFormat,
	dword usageMask,
	ETextureDimension textureDimension,
	s32 textureSlot,
	EImageFormat expectedDataFormat,
	const std::filesystem::path& filePath
) : iObject(context, poolIndex)
{
	SImageBackend tex = _context->GetBackend<IImageDecoder>()->Decode(
		expectedDataFormat,
		filePath
	);

	CreateOnGpu(
		textureFormat,
		usageMask,
		textureDimension,
		tex.size,
		tex.data,
		textureSlot
	);

	GenerateMips();
}

triton::XTexture::XTexture(
	cContext* context,
	s32 poolIndex,
	ETextureFormat textureFormat,
	dword usageMask,
	ETextureDimension textureDimension,
	s32 textureSlot,
	EImageFileFormat containerFormat,
	EImageFormat expectedDataFormat,
	const u8* data,
	usize dataByteSize
) : iObject(context, poolIndex)
{
	SImageBackend tex = _context->GetBackend<IImageDecoder>()->Decode(
		containerFormat,
		expectedDataFormat,
		data,
		dataByteSize
	);

	CreateOnGpu(
		textureFormat,
		usageMask,
		textureDimension,
		tex.size,
		tex.data,
		textureSlot
	);

	GenerateMips();
}

triton::XTexture::XTexture(
	cContext* context,
	s32 poolIndex,
	ETextureFormat textureFormat,
	dword usageMask,
	ETextureDimension textureDimension,
	const cVector3& size,
	const types::u8* data,
	s32 textureSlot
) : iObject(context, poolIndex)
{
	CreateOnGpu(
		textureFormat,
		usageMask,
		textureDimension,
		size,
		data,
		textureSlot
	);

	GenerateMips();
}

triton::XTexture::~XTexture()
{
	if (_gpuTexture.GetFormat() == ETextureFormat::Unknown)
		return;

	_context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
		ERenderCommand::DESTROY_TEXTURE,
		(cpuword)&_gpuTexture
	));

	_context->GetSubsystem<CEngine>()->
		GetSynchronization()->
		WaitForRenderCommandResult<void*>();
}

void triton::XTexture::CreateOnGpu(
	ETextureFormat format,
	dword usageMask,
	ETextureDimension dimension,
	const cVector3& size,
	const u8* data,
	s32 slot
)
{
	_context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
		ERenderCommand::CREATE_TEXTURE,
		(cpuword)format,
		(cpuword)usageMask,
		(cpuword)dimension,
		(cpuword)size.GetX(),
		(cpuword)size.GetY(),
		(cpuword)size.GetZ(),
		(cpuword)data,
		(cpuword)slot
	));

	_gpuTexture =
		_context->GetSubsystem<CEngine>()->
		GetSynchronization()->
		WaitForRenderCommandResult<CGPUTextureResource>();
}

void triton::XTexture::GenerateMips()
{
	if (_gpuTexture.GetFormat() == ETextureFormat::RGBA8_SRGB_Mips)
	{
		_context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
			ERenderCommand::GENERATE_TEXTURE_MIPS,
			(cpuword)&_gpuTexture
		));

		_context->GetSubsystem<CEngine>()->
			GetSynchronization()->
			WaitForRenderCommandResult<void*>();
	}
}