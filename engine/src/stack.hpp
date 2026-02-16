// stack.hpp

#pragma once

#include <type_traits>
#include "object.hpp"
#include "context.hpp"
#include "memory_pool.hpp"
#include "stack_value.hpp"
#include "types.hpp"

namespace triton
{
	struct sChunkAllocatorDescriptor
	{
		types::usize chunkByteSize = 16 * 1024;
		types::usize maxChunkCount = 256;
		types::usize hashTableSize = 4096;
	};

	template <typename TValue>
	class cStack : public iObject
	{
		TRITON_OBJECT(cStack)

	public:
		static_assert(std::is_base_of_v<cStackValue, TValue>, "TValue must inherit from cStackValue");

		explicit cStack(cContext* context, const sChunkAllocatorDescriptor& allocatorDesc);
		virtual ~cStack() override final;

		template<typename... Args>
		TValue* Push(Args&&... args);
		TValue* Push(TValue&& value);
		TValue* At(types::u32 index) const;
		TValue* At(const cStackValue& value) const;
		TValue* Top() const;
		void Erase(types::u32 index);
		void Pop();

		inline types::usize GetSize() const { return _elementCount; }

	private:
		cStackValue New();
		types::u32 AllocateChunk();
		void DeallocateChunk(types::u32 chunkIndex);
		types::u32 GetChunkIndex(types::u32 globalPosition) const;
		types::u32 GetChunkLocalPosition(types::u32 chunkIndex, types::u32 globalPosition) const;

	private:
		sChunkAllocatorDescriptor _allocatorDesc = {};
		types::usize _chunkCount = 0;
		types::usize _objectByteSize = 0;
		types::usize _objectCountPerChunk = 0;
		types::usize _elementCount = 0;
		TValue** _chunks = nullptr;
	};
}

template <typename TValue>
triton::cStack<TValue>::cStack(cContext* context, const sChunkAllocatorDescriptor& allocatorDesc) : iObject(context)
{
	const sCapabilities* caps = _context->GetSubsystem<cEngine>()->GetCapabilities();
	cMemoryAllocator* memoryAllocator = _context->GetMemoryAllocator();

	_allocatorDesc = allocatorDesc;
	_objectByteSize = sizeof(TValue);
	_objectCountPerChunk = _allocatorDesc.chunkByteSize / _objectByteSize;
	_chunks = (TValue**)memoryAllocator->Allocate(_allocatorDesc.maxChunkCount * sizeof(TValue*), caps->memoryAlignment);

	AllocateChunk();
}

template <typename TValue>
triton::cStack<TValue>::~cStack()
{
	for (types::usize i = 0; i < _elementCount; i++)
		Pop();

	cMemoryAllocator* memoryAllocator = _context->GetMemoryAllocator();
	memoryAllocator->Deallocate(_chunks);
}

template <typename TValue>
template <typename... Args>
TValue* triton::cStack<TValue>::Push(Args&&... args)
{
	cStackValue se = New();

	TValue* object = _context->Create<TValue>((types::u8*)_chunks[se.chunk], se.localPosition, std::forward<Args>(args)...);
	object->chunk = se.chunk;
	object->localPosition = se.localPosition;
	object->globalPosition = se.globalPosition;

	return object;
}

template <typename TValue>
TValue* triton::cStack<TValue>::Push(TValue&& value)
{
	cStackValue se = New();

	_chunks[se.chunk][se.localPosition] = std::move(value);

	TValue* object = &_chunks[se.chunk][se.localPosition];
	object->chunk = se.chunk;
	object->localPosition = se.localPosition;
	object->globalPosition = se.globalPosition;

	return object;
}

template <typename TValue>
TValue* triton::cStack<TValue>::At(types::u32 index) const
{
	if (_chunkCount == 0 || index >= _elementCount)
		return nullptr;

	const types::u32 chunkIndex = GetChunkIndex(index);
	const types::u32 localPosition = GetChunkLocalPosition(chunkIndex, index);

	return &_chunks[chunkIndex][localPosition];
}

template <typename TValue>
TValue* triton::cStack<TValue>::At(const cStackValue& value) const
{
	return At(value.globalPosition);
}

template <typename TValue>
TValue* triton::cStack<TValue>::Top() const
{
	if (_elementCount == 0)
		return nullptr;

	return At(_elementCount - 1);
}

template <typename TValue>
void triton::cStack<TValue>::Erase(types::u32 index)
{
	if (_elementCount == 0 || index >= _elementCount)
		return;

	const types::u32 lastChunkIndex = _chunkCount - 1;
	const types::u32 chunkIndex = GetChunkIndex(index);
	const types::usize lastChunkObjectCount = GetChunkLocalPosition(lastChunkIndex, _elementCount);
	const types::u32 lastLocalPosition = lastChunkObjectCount - 1;
	const types::usize localPosition = GetChunkLocalPosition(chunkIndex, index);
	const types::boolean isLastChunk = chunkIndex == lastChunkIndex;

	if (chunkIndex >= _chunkCount || (isLastChunk == types::K_TRUE && localPosition >= lastChunkObjectCount))
		return;

	_chunks[chunkIndex][localPosition].~TValue();
	_chunks[chunkIndex][localPosition] = _chunks[lastChunkIndex][lastLocalPosition];
	_elementCount -= 1;

	if (lastChunkObjectCount == 1)
		DeallocateChunk(lastChunkIndex);
}

template <typename TValue>
void triton::cStack<TValue>::Pop()
{
	Erase(_elementCount - 1);
}

template <typename TValue>
triton::cStackValue triton::cStack<TValue>::New()
{
	types::u32 elementChunkIndex = GetChunkIndex(_elementCount);
	types::u32 elementLocalPosition = GetChunkLocalPosition(elementChunkIndex, _elementCount);

	const types::usize chunkObjectCount = GetChunkLocalPosition(_chunkCount - 1, _elementCount);
	if (chunkObjectCount >= _objectCountPerChunk)
	{
		elementChunkIndex = AllocateChunk();
		elementLocalPosition = 0;
	}

	cStackValue se = {};
	se.chunk = elementChunkIndex;
	se.localPosition = elementLocalPosition;
	se.globalPosition = _elementCount++;

	return se;
}

template <typename TValue>
types::u32 triton::cStack<TValue>::AllocateChunk()
{
	if (_chunkCount >= _allocatorDesc.maxChunkCount)
		return 0;

	const sCapabilities* caps = _context->GetSubsystem<cEngine>()->GetCapabilities();
	cMemoryAllocator* memoryAllocator = _context->GetMemoryAllocator();
	_chunks[_chunkCount] = (TValue*)memoryAllocator->Allocate(_allocatorDesc.chunkByteSize, caps->memoryAlignment);

	return _chunkCount++;
}

template <typename TValue>
void triton::cStack<TValue>::DeallocateChunk(types::u32 chunkIndex)
{
	if (chunkIndex >= _chunkCount--)
		return;

	cMemoryAllocator* memoryAllocator = _context->GetMemoryAllocator();
	memoryAllocator->Deallocate(_chunks[chunkIndex]);
}

template <typename TValue>
types::u32 triton::cStack<TValue>::GetChunkIndex(types::u32 globalPosition) const
{
	return globalPosition / _objectCountPerChunk;
}

template <typename TValue>
types::u32 triton::cStack<TValue>::GetChunkLocalPosition(types::u32 chunkIndex, types::u32 globalPosition) const
{
	const types::u32 chunkIndexBoundary = chunkIndex * _objectCountPerChunk;
	const types::u32 localPosition = globalPosition - chunkIndexBoundary;

	return localPosition;
}