// ecs_subsystem.cpp

#include "ecs_subsystem.hpp"
#include "context.hpp"
#include "engine.hpp"
#include "stack.hpp"

using namespace types;

void triton::ecs::XECSSubsystem::Initialize()
{
	const sCapabilities* caps = _context->GetSubsystem<cEngine>()->GetCapabilities();
	sChunkAllocatorDescriptor cad = {};
	cad.chunkByteSize = caps->hashTableChunkByteSize;
	cad.maxChunkCount = caps->hashTableMaxChunkCount;
	cad.hashTableSize = caps->hashTableSize;
	if (!_sceneGenerationSlots)
		_sceneGenerationSlots = _context->Create<cStack<SSceneSlot>>(_context, cad);
	if (!_scenes)
		_scenes = _context->Create<cStack<cScene>>(_context, cad);
}

void triton::ecs::XECSSubsystem::Shutdown()
{
	if (_sceneGenerationSlots)
		_context->Destroy<cStack<SSceneSlot>>(_sceneGenerationSlots);
	if (_scenes)
		_context->Destroy<cStack<cScene>>(_scenes);
}

triton::SSceneHandle triton::ecs::XECSSubsystem::CreateScene(const std::string& name)
{
	usize arrayIndex = _scenes->GetSize();
	usize slotIndex = _sceneGenerationSlots->GetSize();
	usize generation = 0;

	cScene* scene = _scenes->Push(_context, name);
	SSceneSlot slot = {};
	slot._arrayIndex = arrayIndex;
	slot._generation = generation;
	_sceneGenerationSlots->Push(std::move(slot));

	SSceneHandle handle = {};
	handle._slotIndex = slotIndex;
	handle._generation = generation;

	return handle;
}

triton::cScene* triton::ecs::XECSSubsystem::GetScene(const SSceneHandle& handle)
{
	SSceneSlot* slot = _sceneGenerationSlots->At(handle._slotIndex);
	if (handle._generation != slot->_generation)
		return nullptr;

	return _scenes->At(slot->_arrayIndex);
}

void triton::ecs::XECSSubsystem::DestroyScene(const SSceneHandle& handle)
{
	SSceneSlot* slot = _sceneGenerationSlots->At(handle._slotIndex);
	_scenes->Erase(slot->_arrayIndex);
	slot->_generation += 1;
}