// ecs_subsystem.cpp

#include "ecs_subsystem.hpp"
#include "context.hpp"
#include "engine.hpp"
#include "dynamic_array.hpp"
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
		_handleAllocator = _context->Create<CHandleAllocator<SSceneSlot, cScene, HScene, XDynamicArray<cScene>>>(
			_context,
			4096,
			4096,
			65536 * 64,
			1024
		);
	}
}

void triton::ecs::XECSSubsystem::Shutdown()
{
	if (_handleAllocator)
	{
		_context->Destroy<CHandleAllocator<SSceneSlot, cScene, HScene, XDynamicArray<cScene>>>(_handleAllocator);
	}
}

triton::HScene triton::ecs::XECSSubsystem::CreateScene(const std::string& name)
{
	return HScene();
	//return _handleAllocator->Create(_context, name);
}

triton::ecs::cScene& triton::ecs::XECSSubsystem::GetScene(const HScene& handle)
{
	return _handleAllocator->Get(handle);
}

void triton::ecs::XECSSubsystem::DestroyScene(const HScene& handle)
{
	return _handleAllocator->Destroy(handle);
}