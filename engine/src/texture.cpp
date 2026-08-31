// texture.cpp

#include "texture.hpp"
#include "context.hpp"
#include "synchronization.hpp"
#include "image_decoder.hpp"
#include "graphics_texture_usage_enum.hpp"
#include "object_allocator.hpp"

using namespace types;

triton::XTexture::XTexture(
	cContext* context,
	s32 poolIndex,
	types::boolean bCreateSampler,
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
		bCreateSampler,
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
	types::boolean bCreateSampler,
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
		bCreateSampler,
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
	types::boolean bCreateSampler,
	ETextureFormat textureFormat,
	dword usageMask,
	ETextureDimension textureDimension,
	const cVector3& size,
	const types::u8* data,
	s32 textureSlot
) : iObject(context, poolIndex)
{
	CreateOnGpu(
		bCreateSampler,
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
	boolean bCreateSampler,
	ETextureFormat format,
	dword usageMask,
	ETextureDimension dimension,
	const cVector3& size,
	const u8* data,
	s32 slot
)
{
	cVector3* textureSize = CObjectAllocator::Create<cVector3>(64, size);

	_context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
		ERenderCommand::CREATE_TEXTURE,
		(cpuword)bCreateSampler,
		(cpuword)format,
		(cpuword)usageMask,
		(cpuword)ETextureDimension::Texture2D,
		(cpuword)textureSize,
		(cpuword)&_gpuTexture
	));

	_gpuTexture =
		_context->GetSubsystem<CEngine>()->
		GetSynchronization()->
		WaitForRenderCommandResult<CGPUTextureResource>();

	const usize dataByteSize =
		textureSize->GetX() *
		textureSize->GetY() *
		textureSize->GetZ() *
		TextureFormatToChannelCount(format);

	textureSize->SetX(0.0f);
	textureSize->SetY(0.0f);
	textureSize->SetZ(0.0f);

	const cVector3* writeOffset = textureSize;

	_context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
		ERenderCommand::WRITE_TEXTURE,
		(cpuword)&_gpuTexture,
		(cpuword)writeOffset,
		(cpuword)data,
		(cpuword)dataByteSize
	));

	_context->GetSubsystem<CEngine>()->
		GetSynchronization()->
		WaitForRenderCommandResult<void*>();

	CObjectAllocator::Destroy<cVector3>((cVector3*)textureSize);
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

usize triton::XTexture::TextureFormatToChannelCount(ETextureFormat textureFormat)
{
	if (textureFormat == ETextureFormat::R8)
		return 1;
	else if (textureFormat == ETextureFormat::RGBA8 ||
		textureFormat == ETextureFormat::RGBA8_SRGB ||
		textureFormat == ETextureFormat::RGBA8_SRGB_Mips ||
		textureFormat == ETextureFormat::BGRA8_SRGB ||
		textureFormat == ETextureFormat::DepthStencil)
		return 4;

	return 0;
}