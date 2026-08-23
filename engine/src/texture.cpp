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

	_context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
		ERenderCommand::CREATE_TEXTURE,
		(cpuword)tex.size.GetX(),
		(cpuword)tex.size.GetY(),
		(cpuword)tex.size.GetZ(),
		(cpuword)textureDimension,
		(cpuword)textureFormat,
		(cpuword)tex.data,
		(cpuword)textureSlot
	));

	_gpuTexture =
		_context->GetSubsystem<CEngine>()->
		GetSynchronization()->
		WaitForRenderCommandResult<CGPUTextureResource>();
}

triton::XTexture::XTexture(
	cContext* context,
	s32 poolIndex,
	ETextureFormat textureFormat,
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

	_context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
		ERenderCommand::CREATE_TEXTURE,
		(cpuword)tex.size.GetX(),
		(cpuword)tex.size.GetY(),
		(cpuword)tex.size.GetZ(),
		(cpuword)textureDimension,
		(cpuword)textureFormat,
		(cpuword)tex.data,
		(cpuword)textureSlot
	));

	_gpuTexture =
		_context->GetSubsystem<CEngine>()->
		GetSynchronization()->
		WaitForRenderCommandResult<CGPUTextureResource>();
}

triton::XTexture::XTexture(
	cContext* context,
	s32 poolIndex,
	ETextureFormat textureFormat,
	ETextureDimension textureDimension,
	s32 textureSlot,
	EImageFormat expectedDataFormat,
	const u8* data,
	const cVector3& size
)
{
	_context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
		ERenderCommand::CREATE_TEXTURE,
		(cpuword)size.GetX(),
		(cpuword)size.GetY(),
		(cpuword)size.GetZ(),
		(cpuword)textureDimension,
		(cpuword)textureFormat,
		(cpuword)data,
		(cpuword)textureSlot
	));

	_gpuTexture =
		_context->GetSubsystem<CEngine>()->
		GetSynchronization()->
		WaitForRenderCommandResult<CGPUTextureResource>();
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