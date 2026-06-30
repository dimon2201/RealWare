// material_uploader.cpp

#include "material_uploader.hpp"
#include "context.hpp"
#include "linear_array.hpp"

triton::XMaterialUploader::XMaterialUploader(cContext* context)
{
	const sCapabilities* caps = _context->GetSubsystem<cEngine>()->GetCapabilities();
	sChunkAllocatorDescriptor cad;
	cad.chunkByteSize = caps->hashTableChunkByteSize;
	cad.hashTableSize = caps->hashTableSize;
	cad.maxChunkCount = caps->hashTableMaxChunkCount;
	_buffer = _context->Create<XLinearArray<SMaterialLayout>>(_context, cad);
}

triton::XMaterialUploader::~XMaterialUploader()
{
	_context->Destroy<XLinearArray<SMaterialLayout>>(_buffer);
}