// material_uploader.cpp

#include "material_uploader.hpp"
#include "context.hpp"
#include "linear_array.hpp"
#include "render_subsystem.hpp"
#include "texture_subsystem.hpp"
#include "material.hpp"

using namespace types;

triton::XMaterialUploader::XMaterialUploader(cContext* context) : _context(context)
{
	const sCapabilities* caps = _context->GetSubsystem<cEngine>()->GetCapabilities();
	_stagingBuffer = (SMaterialLayout*)_context->GetMemoryAllocator()->Allocate(caps->maxRenderMaterialCount * sizeof(SMaterialLayout), 64);
}

triton::XMaterialUploader::~XMaterialUploader()
{
	_context->GetMemoryAllocator()->Deallocate(_stagingBuffer);
}

void triton::XMaterialUploader::Set(XTextureSubsystem* textureSubsystem, types::usize index, const SMaterial& material)
{
	const STexture& diffuseTexture = textureSubsystem->Get(material.diffuseTexture);
	STextureLayout diffuseTextureLayout;
	diffuseTextureLayout.layer = diffuseTexture.layer;
	diffuseTextureLayout.normOffset = diffuseTexture.normOffset;
	diffuseTextureLayout.normSize = diffuseTexture.normSize;

	const STexture& normalTexture = textureSubsystem->Get(material.normalTexture);
	STextureLayout normalTextureLayout;
	normalTextureLayout.layer = normalTexture.layer;
	normalTextureLayout.normOffset = normalTexture.normOffset;
	normalTextureLayout.normSize = normalTexture.normSize;

	const STexture& roughnessTexture = textureSubsystem->Get(material.roughnessTexture);
	STextureLayout roughnessTextureLayout;
	roughnessTextureLayout.layer = roughnessTexture.layer;
	roughnessTextureLayout.normOffset = roughnessTexture.normOffset;
	roughnessTextureLayout.normSize = roughnessTexture.normSize;

	const STexture& metallicTexture = textureSubsystem->Get(material.metallicTexture);
	STextureLayout metallicTextureLayout;
	metallicTextureLayout.layer = metallicTexture.layer;
	metallicTextureLayout.normOffset = metallicTexture.normOffset;
	metallicTextureLayout.normSize = metallicTexture.normSize;

	SMaterialLayout materialLayout;
	materialLayout.diffuseColor = material.diffuseColor;
	materialLayout.diffuseTextureLayout = diffuseTextureLayout;
	materialLayout.normalTextureLayout = normalTextureLayout;
	materialLayout.roughnessTextureLayout = roughnessTextureLayout;
	materialLayout.metallicTextureLayout = metallicTextureLayout;

	_stagingBuffer[index] = materialLayout;

	MarkDirty();
}

void triton::XMaterialUploader::MarkDirty()
{
	_bIsDirty = K_TRUE;
}

void triton::XMaterialUploader::Upload(cBuffer* materialBuffer, usize byteSizeToUpload)
{
	if (_bIsDirty == K_TRUE)
	{
		XRenderSubsystem* renderSubsystem = _context->GetSubsystem<XRenderSubsystem>();
		renderSubsystem->PushCommand(SRenderCommand(
			ERenderCommand::WRITE_BUFFER,
			(cpuword)materialBuffer,
			0,
			byteSizeToUpload,
			(cpuword)&_stagingBuffer[0]
		));

		_bIsDirty = K_FALSE;
	}
}