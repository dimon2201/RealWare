// instance_buffer.cpp

#include "instance_buffer.hpp"
#include "capabilities.hpp"
#include "engine.hpp"
#include "context.hpp"
#include "data_buffer.hpp"
#include "components.hpp"

using namespace triton::ecs::components;
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
	_instances = _context->Create<cHashTable<cTag, SInstanceBufferOffset>>(_context, cad);
}

void triton::XInstanceBuffer::Free()
{
	if (_cpuBuffer)
		_context->Destroy<XDataBuffer>(_cpuBuffer);
	if (_instances)
		_context->Destroy<cHashTable<cTag, SInstanceBufferOffset>>(_instances);
}

void triton::XInstanceBuffer::Add(const std::string& tag, SRenderInstance::EUsage usage, const SRenderInstance& instance)
{
	if (!_instances)
		return;

	if (usage == SRenderInstance::EUsage::STATIC)
	{
		_instances->Insert(cTag(tag), SInstanceBufferOffset(_firstDynamicInstanceBytePointer));

		_cpuBuffer->Move(_lastDynamicInstanceBytePointer - _firstDynamicInstanceBytePointer, _firstDynamicInstanceBytePointer, _firstDynamicInstanceBytePointer + sizeof(SRenderInstance));
		_cpuBuffer->Write((const u8*)&instance, sizeof(SRenderInstance), _firstDynamicInstanceBytePointer);
		_firstDynamicInstanceBytePointer += sizeof(SRenderInstance);
		_lastDynamicInstanceBytePointer += sizeof(SRenderInstance);
	}
	else if (usage == SRenderInstance::EUsage::DYNAMIC)
	{
		_instances->Insert(cTag(tag), std::move(_lastDynamicInstanceBytePointer));

		_cpuBuffer->Write((const u8*)&instance, sizeof(SRenderInstance), _lastDynamicInstanceBytePointer);
		_lastDynamicInstanceBytePointer += sizeof(SRenderInstance);
	}
}

void triton::XInstanceBuffer::Remove(const std::string& tag)
{
	if (!_instances)
		return;

	SInstanceBufferOffset* value = _instances->Find(cTag(tag));
	if (value)
	{
		usize byteOffset = value->GetOffset();
		_cpuBuffer->Move(_cpuBuffer->GetByteSize() - (byteOffset + sizeof(SRenderInstance)), byteOffset + sizeof(SRenderInstance), byteOffset);
	}
}

void triton::XInstanceBuffer::UploadStatic(const SRenderData& data)
{
	usize size = data._renderInstances->GetSize();
	for (usize i = 0; i < size; i++)
	{
		SRenderInstanceComponent* ric = data._renderInstances->At(i);
		SRenderInstance ri = {};
		_cpuBuffer->Write((u8*)&ri, sizeof(SRenderInstance), i * sizeof(SRenderInstance));
	}
	_firstDynamicInstanceBytePointer = size * sizeof(SRenderInstance);
	_lastDynamicInstanceBytePointer = _firstDynamicInstanceBytePointer;
}

void triton::XInstanceBuffer::UploadDynamic(const SRenderData& data)
{
	usize size = data._renderInstances->GetSize();
	for (usize i = 0; i < size; i++)
	{
		SRenderInstanceComponent* ric = data._renderInstances->At(i);
		SRenderInstance ri = {};
		_cpuBuffer->Write((u8*)&ri, sizeof(SRenderInstance), _firstDynamicInstanceBytePointer + (i * sizeof(SRenderInstance)));
	}
	_lastDynamicInstanceBytePointer += size * sizeof(SRenderInstance);
}

void triton::XInstanceBuffer::WriteToGPUStatic()
{
	if (!_instances)
		return;

	iGraphicsResourceBackend* gfxResourceBackend = _context->GetBackend<iGraphicsResourceBackend>();
	gfxResourceBackend->WriteBuffer(this, 0, _firstDynamicInstanceBytePointer, _cpuBuffer->GetData());
}

void triton::XInstanceBuffer::WriteToGPUDynamic()
{
	if (!_instances)
		return;

	iGraphicsResourceBackend* gfxResourceBackend = _context->GetBackend<iGraphicsResourceBackend>();
	gfxResourceBackend->WriteBuffer(this, _firstDynamicInstanceBytePointer, _lastDynamicInstanceBytePointer - _firstDynamicInstanceBytePointer, _cpuBuffer->GetData() + _firstDynamicInstanceBytePointer);
}