#pragma once

#include <type_traits>
#include "object.hpp"
#include "engine.hpp"
#include "application.hpp"
#include "hash_table.hpp"
#include "stack.hpp"
#include "entity.hpp"
#include "scene.hpp"
#include "components.hpp"
#include "single_value.hpp"
#include "types.hpp"

namespace triton
{
	class cContext;
}

namespace triton::ecs
{
	template <typename TComponent>
	class cComponentStorage : public iObject
	{
		TRITON_OBJECT(cComponentStorage)

		static_assert(std::is_base_of_v<components::SComponent, TComponent>, "TComponent must inherit from sComponent");

		cStack<TComponent>* _data = nullptr;
		cHashTable<entity, cSingleValue>* _indices = nullptr;

	public:
		explicit cComponentStorage(triton::cContext* context);
		virtual ~cComponentStorage() override final;

		TComponent* Create(entity ent);
		TComponent* Get(entity ent);
		void Destroy(entity ent);
	};
}

template <typename TComponent>
triton::ecs::cComponentStorage<TComponent>::cComponentStorage(triton::cContext* context) : iObject(context)
{
	const sCapabilities* caps = _context->GetSubsystem<cEngine>()->GetApplication()->GetCapabilities();

	sChunkAllocatorDescriptor cad = {};
	cad.chunkByteSize = caps->hashTableChunkByteSize;
	cad.maxChunkCount = caps->hashTableMaxChunkCount;
	cad.hashTableSize = caps->hashTableSize;

	_data = _context->Create<cStack<TComponent>>(_context, cad);
	_indices = _context->Create<cHashTable<entity, cSingleValue>>(_context, cad);
}

template <typename TComponent>
triton::ecs::cComponentStorage<TComponent>::~cComponentStorage()
{
	_context->Destroy<cHashTable<entity, cSingleValue>>(_indices);
	_context->Destroy<cStack<TComponent>>(_data);
}

template <typename TComponent>
TComponent* triton::ecs::cComponentStorage<TComponent>::Create(entity ent)
{
	_indices->Insert(ent, _data->GetSize());
	TComponent component = {};

	return _data->Push(std::move(component));
}

template <typename TComponent>
TComponent* triton::ecs::cComponentStorage<TComponent>::Get(entity ent)
{
	cSingleValue* index = _indices->Find(ent);
	TComponent* component = _data->At(index->Value());

	return component;
}

template <typename TComponent>
void triton::ecs::cComponentStorage<TComponent>::Destroy(entity ent)
{
	cSingleValue* index = _indices->Find(ent);
	_data->Erase(index->Value());
}