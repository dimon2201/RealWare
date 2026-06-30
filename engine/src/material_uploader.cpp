// material_uploader.cpp

#include "material_uploader.hpp"
#include "context.hpp"
#include "linear_array.hpp"

triton::XMaterialUploader::XMaterialUploader(cContext* context) : iObject(context)
{
	const sCapabilities* caps = _context->GetSubsystem<cEngine>()->GetCapabilities();
	_buffer = (SMaterialLayout*)_context->GetMemoryAllocator()->Allocate(caps->maxRenderMaterialCount * sizeof(SMaterialLayout), 64);
}

triton::XMaterialUploader::~XMaterialUploader()
{
	_context->GetMemoryAllocator()->Deallocate(_buffer);
}

void triton::XMaterialUploader::Set(types::usize index, const SMaterial& material)
{
}

void triton::XMaterialUploader::Upload(cBuffer* materialBuffer)
{
}