#pragma once

#include "ecs.hpp"

using namespace triton;
using namespace types;

namespace triton::ecs
{
	cSceneStorage::cSceneStorage(cContext* context) : iObject(context)
	{
		const sCapabilities* caps = _context->GetSubsystem<cEngine>()->GetApplication()->GetCapabilities();

		sChunkAllocatorDescriptor cad = {};
		cad.chunkByteSize = caps->hashTableChunkByteSize;
		cad.maxChunkCount = caps->hashTableMaxChunkCount;
		cad.hashTableSize = caps->hashTableSize;

		_scenes = _context->Create<cStack<cScene>>(_context, cad);
	}

	cSceneStorage::~cSceneStorage()
	{
		_context->Destroy<cStack<cScene>>(_scenes);
	}

	cScene* cSceneStorage::Create(const std::string& name)
	{
		_sceneIndices->Insert(name, _scenes->GetSize());
		cScene* scene = _scenes->Push(_context, name);

		return scene;
	}

	cScene* cSceneStorage::Get(const std::string& name)
	{
		cSingleValue* index = _sceneIndices->Find(name);
		cScene* scene = _scenes->At(index->Value());

		return scene;
	}

	cScene* cSceneStorage::Get(types::usize index)
	{
		return _scenes->At(index);
	}

	void cSceneStorage::Destroy(const std::string& name)
	{
		cSingleValue* index = _sceneIndices->Find(name);
		_scenes->Erase(index->Value());
	}

	cSceneStorage::sFindResult cSceneStorage::FindEntity(entity ent)
	{
		sFindResult fr = {};
		fr.isOK = K_FALSE;

		for (usize i = 0; i < _scenes->GetSize(); i++)
		{
			cScene* scene = _scenes->At(i);
			if (scene->IsEntityExist(ent))
			{
				fr.isOK = K_TRUE;
				fr.index = i;
				
				return fr;
			}
		}

		return fr;
	}
}