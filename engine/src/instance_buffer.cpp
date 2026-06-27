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

triton::XInstanceBuffer::XInstanceBuffer(cContext* context) : cBuffer(context, 0, cBuffer::eType::NONE, 0, 0), _isCpuOnly(K_TRUE)
{
	Initialize();
}

triton::XInstanceBuffer::XInstanceBuffer(cContext* context, cBuffer* buffer) : cBuffer(context, buffer->GetInstance(), buffer->GetBufferType(), buffer->GetByteSize(), buffer->GetSlot()), _isCpuOnly(K_FALSE)
{
	Initialize();
}

triton::XInstanceBuffer::~XInstanceBuffer()
{
	_instances->Free();
	_context->Destroy<XHandleAllocator<SInstanceBufferSlot, SInstanceBufferHandle, XLinearArray<SRenderInstance>, SRenderInstance>>(_instances);
}

void triton::XInstanceBuffer::Initialize()
{
	_instances = _context->Create<XHandleAllocator<SInstanceBufferSlot, SInstanceBufferHandle, XLinearArray<SRenderInstance>, SRenderInstance>>(_context);
	_instances->Initialize();
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
	CThreadGuard::AssertRender();

	_context->GetBackend<iGraphicsResourceBackend>()->WriteBuffer(
		this,
		handle._indexInArray * sizeof(SRenderInstance),
		sizeof(SRenderInstance),
		(const u8*)_instances->Get(handle)
	);
}

void triton::XInstanceBuffer::WriteAll()
{
	CThreadGuard::AssertRender();

	SBufferView bufferView = _instances->GetData();
	if (bufferView._byteSize == 0)
		return;
	_context->GetBackend<iGraphicsResourceBackend>()->WriteBuffer(
		this,
		0,
		bufferView._byteSize,
		(const u8*)bufferView._data
	);
}