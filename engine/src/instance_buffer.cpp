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

triton::SInstanceBufferHandle triton::XInstanceBuffer::Add(const SRenderInstance& instance)
{
	return _instances->Create(std::move(instance));
}

triton::SRenderInstance* triton::XInstanceBuffer::Get(const SInstanceBufferHandle& handle)
{
	return _instances->Get(handle);
}

void triton::XInstanceBuffer::Remove(const SInstanceBufferHandle& handle)
{
	_instances->Destroy(handle);
}

void triton::XInstanceBuffer::Write(const SInstanceBufferHandle& handle)
{
	SRenderCommand cmd = {};
	cmd._command = ERenderCommand::WRITE_BUFFER;
	cmd._args._argA = (cpuword)this;
	cmd._args._argB = handle._indexInArray * sizeof(SRenderInstance);
	cmd._args._argC = sizeof(SRenderInstance);
	cmd._args._argD = (cpuword)_instances->Get(handle);
	_context->GetSubsystem<XRenderSubsystem>()->PushCommand(cmd);
}

void triton::XInstanceBuffer::WriteAll()
{
	SBufferView bufferView = _instances->GetData();
	SRenderCommand cmd = {};
	cmd._command = ERenderCommand::WRITE_BUFFER;
	cmd._args._argA = (cpuword)this;
	cmd._args._argB = 0;
	cmd._args._argC = bufferView._byteSize;
	cmd._args._argD = (cpuword)bufferView._data;
	_context->GetSubsystem<XRenderSubsystem>()->PushCommand(cmd);
}