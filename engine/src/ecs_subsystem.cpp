// ecs_subsystem.cpp

#include "ecs_subsystem.hpp"
#include "context.hpp"
#include "engine.hpp"
#include "stack.hpp"
#include "handle_allocator.hpp"

using namespace types;

void triton::ecs::XECSSubsystem::Initialize()
{
	const sCapabilities* caps = _context->GetSubsystem<cEngine>()->GetCapabilities();
	sChunkAllocatorDescriptor cad = {};
	cad.chunkByteSize = caps->hashTableChunkByteSize;
	cad.maxChunkCount = caps->hashTableMaxChunkCount;
	cad.hashTableSize = caps->hashTableSize;
	if (!_handleAllocator)
	{
		_handleAllocator = _context->Create<XHandleAllocator<SSceneSlot, SSceneHandle, cStack<cScene>, cScene>>(_context);
		_handleAllocator->Initialize();
	}
}

void triton::ecs::XECSSubsystem::Shutdown()
{
	if (_handleAllocator)
	{
		_handleAllocator->Free();
		_context->Destroy<XHandleAllocator<SSceneSlot, SSceneHandle, cStack<cScene>, cScene>>(_handleAllocator);
	}
}

triton::SSceneHandle triton::ecs::XECSSubsystem::CreateScene(const std::string& name)
{
	return _handleAllocator->Create(name);
}

triton::ecs::cScene* triton::ecs::XECSSubsystem::GetScene(const SSceneHandle& handle)
{
	return _handleAllocator->Get(handle);
}

void triton::ecs::XECSSubsystem::DestroyScene(const SSceneHandle& handle)
{
	return _handleAllocator->Destroy(handle);
}