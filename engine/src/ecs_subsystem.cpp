// ecs_subsystem.cpp

#include "ecs_subsystem.hpp"
#include "context.hpp"
#include "engine.hpp"
#include "stack.hpp"
#include "handle_table.hpp"

using namespace types;

void triton::ecs::XECSSubsystem::Initialize()
{
	const sCapabilities* caps = _context->GetSubsystem<cEngine>()->GetCapabilities();
	sChunkAllocatorDescriptor cad = {};
	cad.chunkByteSize = caps->hashTableChunkByteSize;
	cad.maxChunkCount = caps->hashTableMaxChunkCount;
	cad.hashTableSize = caps->hashTableSize;
	if (!_handleTable)
	{
		_handleTable = _context->Create<XHandleTable<SSceneSlot, SSceneHandle, cScene>>(_context);
		_handleTable->Initialize();
	}
}

void triton::ecs::XECSSubsystem::Shutdown()
{
	if (_handleTable)
	{
		_handleTable->Free();
		_context->Destroy<XHandleTable<SSceneSlot, SSceneHandle, cScene>>(_handleTable);
	}
}

triton::SSceneHandle triton::ecs::XECSSubsystem::CreateScene(const std::string& name)
{
	return _handleTable->Create(name);
}

triton::ecs::cScene* triton::ecs::XECSSubsystem::GetScene(const SSceneHandle& handle)
{
	return _handleTable->Get(handle);
}

void triton::ecs::XECSSubsystem::DestroyScene(const SSceneHandle& handle)
{
	return _handleTable->Destroy(handle);
}