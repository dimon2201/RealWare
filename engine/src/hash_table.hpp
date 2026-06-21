// hash_table.hpp

#pragma once

#include <type_traits>
#include "application.hpp"
#include "object.hpp"
#include "context.hpp"
#include "memory_pool.hpp"
#include "stack.hpp"
#include "tag.hpp"
#include "math.hpp"
#include "types.hpp"

namespace triton
{
	template <typename TKey, typename TValue>
	class cHashTable;

	template <typename TKey, typename TValue>
	class cHashTablePair : public iObject
	{
		TRITON_OBJECT(cHashTablePair)

		friend class cHashTable<TKey, TValue>;

		TKey _key = {};
		TValue _value = {};

	public:
		explicit cHashTablePair(cContext* context, const TKey& key, TValue&& value);
		virtual ~cHashTablePair() override final = default;
	};

	template <typename TKey, typename TValue>
	class cHashTable : public iObject
	{
		TRITON_OBJECT(cHashTable)

		sChunkAllocatorDescriptor _allocatorDesc = {};
		cStack<cHashTablePair<TKey, TValue>>* _elements;
		types::usize _hashTableSize = 0;
		types::qword _hashMask = 0;
		SStackValue<cHashTablePair<TKey, TValue>>* _hashTableElements = nullptr;

		void HashElement(const SStackValue<cHashTablePair<TKey, TValue>>& element);

	public:
		explicit cHashTable(cContext* context, const sChunkAllocatorDescriptor& allocatorDesc);
		virtual ~cHashTable() override final;

		//template<typename... Args>
		//TValue* Insert(const TKey& key, Args&&... args);
		TValue* Insert(const TKey& key, TValue&& value);
		TValue* Find(const TKey& key) const;
		TValue* Find(types::u32 index) const;
		void Erase(const TKey& key);
		void Erase(types::u32 index);

		inline types::usize GetSize() const { return _elements->GetSize(); }
	};
}

template <typename TKey, typename TValue>
triton::cHashTablePair<TKey, TValue>::cHashTablePair(cContext* context, const TKey& key, TValue&& value)
	: iObject(context), _key(key), _value(std::move(value)) {}

template <typename TKey, typename TValue>
triton::cHashTable<TKey, TValue>::cHashTable(cContext* context, const sChunkAllocatorDescriptor& allocatorDesc) : iObject(context)
{
	const sCapabilities* caps = _context->GetSubsystem<cEngine>()->GetApplication()->GetCapabilities();
	cMemoryAllocator* memoryAllocator = _context->GetMemoryAllocator();

	_allocatorDesc = allocatorDesc;
	_elements = _context->Create<cStack<cHashTablePair<TKey, TValue>>>(_context, _allocatorDesc);
	_hashTableSize = _allocatorDesc.hashTableSize;
	_hashMask = cMath::MakeHashMask(_allocatorDesc.hashTableSize);
	_hashTableElements = (SStackValue<cHashTablePair<TKey, TValue>>*)memoryAllocator->Allocate(_hashTableSize * sizeof(SStackValue<cHashTablePair<TKey, TValue>>), caps->memoryAlignment);
	memset(&_hashTableElements[0], 0, _hashTableSize * sizeof(SStackValue<cHashTablePair<TKey, TValue>>));
}

template <typename TKey, typename TValue>
triton::cHashTable<TKey, TValue>::~cHashTable()
{
	cMemoryAllocator* memoryAllocator = _context->GetMemoryAllocator();
	memoryAllocator->Deallocate(_hashTableElements);

	_context->Destroy<cStack<cHashTablePair<TKey, TValue>>>(_elements);
}

// TODO: Figure out better way
/*template <typename TKey, typename TValue>
template <typename... Args>
TValue* cHashTable<TKey, TValue>::Insert(const TKey& key, Args&&... args)
{
	TValue value(std::forward<Args>(args)...);
	cHashTablePair<TKey, TValue> pair(_context, key, std::move(value));
	cHashTablePair<TKey, TValue>* pPair = _elements->Push(std::move(pair));

	if (pPair == nullptr)
		return nullptr;

	TValue* object = &pPair->_value;

	HashPair(key, object);

	return object;
}*/

template <typename TKey, typename TValue>
TValue* triton::cHashTable<TKey, TValue>::Insert(const TKey& key, TValue&& value)
{
	cHashTablePair<TKey, TValue> pair(_context, std::move(key), std::move(value));
	SStackValue<cHashTablePair<TKey, TValue>> element = _elements->Push(std::move(pair));
	if (element.data == nullptr)
		return nullptr;

	TValue* object = &element.data->_value;
	HashElement(element);

	return object;
}

template <typename TKey, typename TValue>
TValue* triton::cHashTable<TKey, TValue>::Find(const TKey& key) const
{
	const types::qword hash = cMath::Hash<TKey>(key, _hashMask);
	const SStackValue<cHashTablePair<TKey, TValue>>& value = _hashTableElements[hash];
	cHashTablePair<TKey, TValue>* pair = _elements->At(*value.index).data;
	if (pair != nullptr && key == pair->_key)
		return &pair->_value;

	for (types::usize i = 0; i < _elements->GetSize(); i++)
	{
		cHashTablePair<TKey, TValue>* pair = _elements->At(i).data;
		if (pair != nullptr && key == pair->_key)
			return &pair->_value;
	}

	return nullptr;
}

template <typename TKey, typename TValue>
TValue* triton::cHashTable<TKey, TValue>::Find(types::u32 index) const
{
	cHashTablePair<TKey, TValue>* pair = _elements->At(index).data;

	if (pair == nullptr)
		return nullptr;
	else
		return &pair->_value;
}

template <typename TKey, typename TValue>
void triton::cHashTable<TKey, TValue>::Erase(const TKey& key)
{
	const types::qword hash = cMath::Hash<TKey>(key, _hashMask);
	const SStackValue<cHashTablePair<TKey, TValue>>& value = _hashTableElements[hash];
	const cHashTablePair<TKey, TValue>* pair = _elements->At(*value.index).data;
	if (pair != nullptr && key == pair->_key)
		_elements->Erase(value.index->globalPosition);

	for (types::usize i = 0; i < _elements->GetSize(); i++)
	{
		const cHashTablePair<TKey, TValue>* pair = _elements->At(i).data;
		if (pair != nullptr && key == pair->_key)
			_elements->Erase(i);
	}
}

template <typename TKey, typename TValue>
void triton::cHashTable<TKey, TValue>::Erase(types::u32 index)
{
	_elements->Erase(index);
}

template <typename TKey, typename TValue>
void triton::cHashTable<TKey, TValue>::HashElement(const SStackValue<cHashTablePair<TKey, TValue>>& element)
{
	const types::u32 hash = cMath::Hash<TKey>(element.data->_key, _hashMask);
	if (_hashTableElements[hash].index == nullptr)
	{
		// Insert new element
		_hashTableElements[hash] = element;
		return;
	}
	else
	{
		// Rewrite hash table element
		SStackIndex si = {};
		si.chunkIndex = element.index->chunkIndex;
		si.localPosition = element.index->localPosition;
		si.globalPosition = element.index->globalPosition;

		*_hashTableElements[hash].index = si;
	}
}