// instance_buffer.cpp

#include "instance_buffer.hpp"
#include "capabilities.hpp"
#include "engine.hpp"
#include "context.hpp"
#include "data_buffer.hpp"
#include "components.hpp"
#include "handle_allocator.hpp"

using namespace triton::ecs::components;
using namespace types;

void triton::XInstanceBuffer::Initialize()
{
	if (_cpuBuffer)
		return;
	_cpuBuffer = _context->Create<XDataBuffer>(_context, _byteSize);
}

void triton::XInstanceBuffer::Free()
{
	if (_cpuBuffer)
		_context->Destroy<XDataBuffer>(_cpuBuffer);
}

triton::SInstanceBufferHandle triton::XInstanceBuffer::Add(const SRenderInstance& instance)
{
	if (instance._usage == SRenderInstance::EUsage::STATIC)
	{
		SInstanceBufferHandle handle = _handles->Create(_firstDynamicInstanceBytePointer);

		u8* data = _cpuBuffer->GetData();
		const SRenderInstance firstDynamicInstance = *(const SRenderInstance*)(data + _firstDynamicInstanceBytePointer);
		*(SRenderInstance*)(data + _lastDynamicInstanceBytePointer) = firstDynamicInstance;
		*(SRenderInstance*)(data + _firstDynamicInstanceBytePointer) = instance;
		// TODO: write two instances to GPU
		// WriteToGPU(_firstDynamicInstanceBytePointer)
		// WriteToGPU(_lastDynamicInstanceBytePointer)
		_firstDynamicInstanceBytePointer += sizeof(SRenderInstance);
		_lastDynamicInstanceBytePointer += sizeof(SRenderInstance);

		return offset;
	}
	else if (instance._usage == SRenderInstance::EUsage::DYNAMIC)
	{
		SInstanceBufferOffset offset(_lastDynamicInstanceBytePointer);

		u8* data = _cpuBuffer->GetData();
		*(SRenderInstance*)(data + _lastDynamicInstanceBytePointer) = instance;
		// TODO: write instance to GPU
		// WriteToGPU(_lastDynamicInstanceBytePointer)
		_lastDynamicInstanceBytePointer += sizeof(SRenderInstance);

		return offset;
	}

	return SInstanceBufferHandle();
}

triton::SRenderInstance* triton::XInstanceBuffer::Get(const SInstanceBufferOffset& offset)
{
	return (SRenderInstance*)(_cpuBuffer->GetData() + offset.GetOffset());
}

void triton::XInstanceBuffer::Remove(const SInstanceBufferOffset& offset)
{
	usize byteOffset = offset.GetOffset();
	_cpuBuffer->Move(_cpuBuffer->GetByteSize() - (byteOffset + sizeof(SRenderInstance)), byteOffset + sizeof(SRenderInstance), byteOffset);
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
	iGraphicsResourceBackend* gfxResourceBackend = _context->GetBackend<iGraphicsResourceBackend>();
	gfxResourceBackend->WriteBuffer(this, 0, _firstDynamicInstanceBytePointer, _cpuBuffer->GetData());
}

void triton::XInstanceBuffer::WriteToGPUDynamic()
{
	iGraphicsResourceBackend* gfxResourceBackend = _context->GetBackend<iGraphicsResourceBackend>();
	gfxResourceBackend->WriteBuffer(this, _firstDynamicInstanceBytePointer, _lastDynamicInstanceBytePointer - _firstDynamicInstanceBytePointer, _cpuBuffer->GetData() + _firstDynamicInstanceBytePointer);
}