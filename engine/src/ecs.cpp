#pragma once

#include "ecs.hpp"

using namespace triton;
using namespace types;

triton::ecs::cSceneStorage::cSceneStorage(cContext* context) : iObject(context)
{
	const sCapabilities* caps = _context->GetSubsystem<cEngine>()->GetApplication()->GetCapabilities();

	sChunkAllocatorDescriptor cad = {};
	cad.chunkByteSize = caps->hashTableChunkByteSize;
	cad.maxChunkCount = caps->hashTableMaxChunkCount;
	cad.hashTableSize = caps->hashTableSize;

	_scenes = _context->Create<XDynamicArray<cScene>>(_context, cad);
}

triton::ecs::cSceneStorage::~cSceneStorage()
{
	_context->Destroy<XDynamicArray<cScene>>(_scenes);
}

const triton::ecs::cScene* triton::ecs::cSceneStorage::Create(const std::string& name)
{
	_sceneIndices->Insert(name, _scenes->GetSize());

	return _scenes->Push(_context, name).data;
}

const triton::ecs::cScene* triton::ecs::cSceneStorage::Get(const std::string& name)
{
	cSingleValue* index = _sceneIndices->Find(name);

	return _scenes->At(index->Value()).data;
}

const triton::ecs::cScene* triton::ecs::cSceneStorage::Get(types::usize index)
{
	return _scenes->At(index).data;
}

void triton::ecs::cSceneStorage::Destroy(const std::string& name)
{
	cSingleValue* index = _sceneIndices->Find(name);
	_scenes->Erase(index->Value());
}

triton::ecs::cSceneStorage::sFindResult triton::ecs::cSceneStorage::FindEntity(entity ent)
{
	sFindResult fr = {};
	fr.isOK = K_FALSE;

	for (usize i = 0; i < _scenes->GetSize(); i++)
	{
		// TODO: remove this const cast when handle storage system will be created
		cScene* scene = (cScene*)_scenes->At(i).data;
		if (scene->IsEntityExist(ent))
		{
			fr.isOK = K_TRUE;
			fr.index = i;
				
			return fr;
		}
	}

	return fr;
}