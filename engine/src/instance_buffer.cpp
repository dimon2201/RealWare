// instance_buffer.cpp

#include "instance_buffer.hpp"
#include "capabilities.hpp"
#include "engine.hpp"
#include "context.hpp"
#include "data_buffer.hpp"

using namespace types;

void triton::XInstanceBuffer::Initialize()
{
	if (_cpuBuffer)
		return;
	_cpuBuffer = _context->Create<XDataBuffer>(_context, _byteSize);

	if (_instances)
		return;
	const sCapabilities* caps = _context->GetSubsystem<cEngine>()->GetCapabilities();
	sChunkAllocatorDescriptor cad = {};
	cad.chunkByteSize = caps->hashTableChunkByteSize;
	cad.hashTableSize = caps->hashTableSize;
	cad.maxChunkCount = caps->hashTableMaxChunkCount;
	_instances = _context->Create<cHashTable<std::string, usize>>(_context, cad);
}

void triton::XInstanceBuffer::Free()
{
	if (_cpuBuffer)
		_context->Destroy<XDataBuffer>(_cpuBuffer);
	if (_instances)
		_context->Destroy<cHashTable<std::string, usize>>(_instances);
}

void triton::XInstanceBuffer::AddInstance(const std::string& tag, SRenderInstance::EUsage usage, const SRenderInstance& instance)
{
	if (!_instances)
		return;

	if (usage == SRenderInstance::EUsage::STATIC)
	{
	}
	else if (usage == SRenderInstance::EUsage::DYNAMIC)
	{
	}
}

void triton::XInstanceBuffer::RemoveInstance(const std::string& tag)
{
	if (!_instances)
		return;
}

void triton::XInstanceBuffer::Write()
{
}