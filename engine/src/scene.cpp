// scene.cpp

#pragma once

#include "scene.hpp"
#include "context.hpp"

using namespace types;

triton::ecs::cScene::cScene(cContext* context, const std::string& name) : iObject(context), _name(name)
{
	const sCapabilities* caps = _context->GetSubsystem<cEngine>()->GetApplication()->GetCapabilities();

	sChunkAllocatorDescriptor cad = {};
	cad.chunkByteSize = caps->hashTableChunkByteSize;
	cad.maxChunkCount = caps->hashTableMaxChunkCount;
	cad.hashTableSize = caps->hashTableSize;

	_isEntityExist = _context->Create<cHashTable<entity, cSingleValue>>(_context, cad);
	_audios = _context->Create<cComponentStorage<components::sSoundComponent>>(_context);
}

triton::ecs::cScene::~cScene()
{
	_context->Destroy<cHashTable<entity, cSingleValue>>(_isEntityExist);
	_context->Destroy<cComponentStorage<components::sSoundComponent>>(_audios);
}

triton::ecs::entity triton::ecs::cScene::CreateEntity()
{
	entity ent = sLastEntity++;

	_isEntityExist->Insert(ent, K_TRUE);

	return ent;
}

void triton::ecs::cScene::DestroyEntity(entity ent)
{
	if (ent != kInvalidEntity)
		_isEntityExist->Erase(ent);
}

types::boolean triton::ecs::cScene::IsEntityExist(entity ent)
{
	cSingleValue* result = _isEntityExist->Find(ent);
	if (result == nullptr)
		return K_FALSE;
	else
		return result->Value();
}