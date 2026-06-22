// instance_buffer.cpp

#include "instance_buffer.hpp"
#include "capabilities.hpp"
#include "engine.hpp"
#include "context.hpp"
#include "data_buffer.hpp"
#include "components.hpp"
#include "handle_allocator.hpp"
#include "buffer_view.hpp"
#include "render_subsystem.hpp"

using namespace triton::ecs::components;
using namespace types;

void triton::XInstanceBuffer::Initialize()
{
	if (!_instances)
		_instances = _context->Create<XHandleAllocator<SInstanceBufferSlot, SInstanceBufferHandle, XLinearArray<SRenderInstance>, SRenderInstance>>(_context);
}

void triton::XInstanceBuffer::Free()
{
	if (_instances)
		_context->Destroy<XHandleAllocator<SInstanceBufferSlot, SInstanceBufferHandle, XLinearArray<SRenderInstance>, SRenderInstance>>(_instances);
}

triton::SInstanceBufferHandle triton::XInstanceBuffer::Add(SRenderInstance& instance)
{
	return _instances->Create(std::move(instance));
}

triton::SRenderInstance* triton::XInstanceBuffer::Get(SInstanceBufferHandle& handle)
{
	return _instances->Get(handle);
}

void triton::XInstanceBuffer::Remove(SInstanceBufferHandle& handle)
{
	_instances->Destroy(handle);
}

void triton::XInstanceBuffer::Write(const SInstanceBufferHandle& handle)
{
	SRenderCommand cmd = SRenderCommand(
		ERenderCommand::WRITE_BUFFER,
		(cpuword)this,
		handle._indexInArray * sizeof(SRenderInstance),
		sizeof(SRenderInstance),
		(cpuword)_instances->Get(handle)
	);
	_context->GetSubsystem<XRenderSubsystem>()->PushCommand(cmd);
}

void triton::XInstanceBuffer::WriteAll()
{
	SBufferView bufferView = _instances->GetData();
	SRenderCommand cmd = SRenderCommand(
		ERenderCommand::WRITE_BUFFER,
		(cpuword)this,
		0,
		bufferView._byteSize,
		(cpuword)bufferView._data
	);
	_context->GetSubsystem<XRenderSubsystem>()->PushCommand(cmd);
}