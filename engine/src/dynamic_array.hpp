// dynamic_array.hpp

#pragma once

#include <type_traits>
#include "object.hpp"
#include "engine.hpp"
#include "capabilities.hpp"
#include "context.hpp"
#include "memory_pool.hpp"
#include "stack_value.hpp"
#include "thread_guard.hpp"
#include "types.hpp"

namespace triton
{
	struct sChunkAllocatorDescriptor
	{
		types::usize chunkByteSize = 16 * 1024;
		types::usize maxChunkCount = 256;
		types::usize hashTableSize = 4096;
	};

	struct SStackIndex final
	{
		types::u32 chunkIndex = 0;
		types::u32 localPosition = 0;
		types::u32 globalPosition = 0;
	};

	template <typename TValue>
	struct SStackValue final
	{
		TValue* data = nullptr;
		SStackIndex* index = nullptr;
	};

	template <typename TValue>
	class XDynamicArray : public iObject
	{
		TRITON_OBJECT(XDynamicArray)

		sChunkAllocatorDescriptor _allocatorDesc = {};
		types::usize _chunkCount = 0;
		types::usize _objectByteSize = 0;
		types::usize _objectCountPerChunk = 0;
		types::usize _elementCount = 0;
		TValue** _chunkValues = nullptr;
		SStackIndex** _chunkIndices = nullptr;

		SStackIndex New();
		types::u32 AllocateChunk();
		void DeallocateChunk(types::u32 chunkIndex);
		types::u32 GetChunkIndex(types::u32 globalPosition) const;
		types::u32 GetChunkLocalPosition(types::u32 chunkIndex, types::u32 globalPosition) const;

	public:
		explicit XDynamicArray(cContext* context, const sChunkAllocatorDescriptor& allocatorDesc);
		virtual ~XDynamicArray() override final;

		template<typename... Args>
		SStackValue<TValue> Push(Args&&... args);
		SStackValue<TValue> Push(TValue&& value);
		SStackValue<TValue> At(types::u32 index) const;
		SStackValue<TValue> At(const SStackIndex& index) const;
		SStackValue<TValue> Top() const;
		void Erase(types::u32 index);
		SStackValue<TValue> Pop();
		void Clear();
		types::boolean IsEmpty();

		inline types::usize GetSize() const { return _elementCount; }
	};

	template <typename TValue>
	XDynamicArray<TValue>::XDynamicArray(cContext* context, const sChunkAllocatorDescriptor& allocatorDesc) : iObject(context)
	{
		const sCapabilities* caps = _context->GetSubsystem<cEngine>()->GetCapabilities();
		cMemoryAllocator* memoryAllocator = _context->GetMemoryAllocator();

		_allocatorDesc = allocatorDesc;
		_objectByteSize = sizeof(TValue);
		_objectCountPerChunk = _allocatorDesc.chunkByteSize / _objectByteSize;
		_chunkValues = (TValue**)memoryAllocator->Allocate(_allocatorDesc.maxChunkCount * sizeof(TValue*), caps->memoryAlignment);
		_chunkIndices = (SStackIndex**)memoryAllocator->Allocate(_allocatorDesc.maxChunkCount * sizeof(SStackIndex*), caps->memoryAlignment);

		AllocateChunk();
	}

	template <typename TValue>
	XDynamicArray<TValue>::~XDynamicArray()
	{
		for (types::usize i = 0; i < _elementCount; i++)
			Pop();

		cMemoryAllocator* memoryAllocator = _context->GetMemoryAllocator();
		for (types::usize i = 0; i < _allocatorDesc.maxChunkCount; i++)
			memoryAllocator->Deallocate(_chunkIndices[i]);

		memoryAllocator->Deallocate(_chunkIndices);
		memoryAllocator->Deallocate(_chunkValues);
	}

	template <typename TValue>
	template <typename... Args>
	SStackValue<TValue> XDynamicArray<TValue>::Push(Args&&... args)
	{
		SStackIndex si = New();
		TValue* object = _context->Create<TValue>(_chunkValues[si.chunkIndex], si.localPosition, std::forward<Args>(args)...);
		_chunkIndices[si.chunkIndex][si.localPosition] = si;

		SStackValue<TValue> returnValue = {};
		returnValue.data = object;
		returnValue.index = &_chunkIndices[si.chunkIndex][si.localPosition];

		return returnValue;
	}

	template <typename TValue>
	SStackValue<TValue> XDynamicArray<TValue>::Push(TValue&& value)
	{
		SStackIndex si = New();
		_chunkValues[si.chunkIndex][si.localPosition] = std::move(value);
		TValue* object = &_chunkValues[si.chunkIndex][si.localPosition];
		_chunkIndices[si.chunkIndex][si.localPosition] = si;

		SStackValue<TValue> returnValue = {};
		returnValue.data = object;
		returnValue.index = &_chunkIndices[si.chunkIndex][si.localPosition];

		return returnValue;
	}

	template <typename TValue>
	SStackValue<TValue> XDynamicArray<TValue>::At(types::u32 index) const
	{
		if (_chunkCount == 0 || index >= _elementCount)
			return {};

		const types::u32 chunkIndex = GetChunkIndex(index);
		const types::u32 localPosition = GetChunkLocalPosition(chunkIndex, index);

		SStackValue<TValue> returnValue = {};
		returnValue.data = &_chunkValues[chunkIndex][localPosition];
		returnValue.index = &_chunkIndices[chunkIndex][localPosition];

		return returnValue;
	}

	template <typename TValue>
	SStackValue<TValue> XDynamicArray<TValue>::At(const SStackIndex& index) const
	{
		return At(index.globalPosition);
	}

	template <typename TValue>
	SStackValue<TValue> XDynamicArray<TValue>::Top() const
	{
		if (_elementCount == 0)
			return {};

		return At(_elementCount - 1);
	}

	template <typename TValue>
	void XDynamicArray<TValue>::Erase(types::u32 index)
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

		_chunkValues[chunkIndex][localPosition].~TValue();
		new (&_chunkValues[chunkIndex][localPosition]) TValue(_chunkValues[lastChunkIndex][lastLocalPosition]);
		_chunkIndices[chunkIndex][localPosition] = _chunkIndices[lastChunkIndex][lastLocalPosition];
		_elementCount -= 1;

		if (lastChunkObjectCount == 1)
			DeallocateChunk(lastChunkIndex);
	}

	template <typename TValue>
	SStackValue<TValue> XDynamicArray<TValue>::Pop()
	{
		SStackValue<TValue> value = Top();
		Erase(_elementCount - 1);

		return value;
	}

	template <typename TValue>
	void XDynamicArray<TValue>::Clear()
	{
		types::usize size = GetSize();
		for (types::usize i = 0; i < size; i++)
			Pop();
	}

	template <typename TValue>
	types::boolean XDynamicArray<TValue>::IsEmpty()
	{
		return _elementCount == 0 ? types::K_TRUE : types::K_FALSE;
	}

	template <typename TValue>
	SStackIndex XDynamicArray<TValue>::New()
	{
		types::u32 elementChunkIndex = GetChunkIndex(_elementCount);
		types::u32 elementLocalPosition = GetChunkLocalPosition(elementChunkIndex, _elementCount);

		const types::usize chunkObjectCount = GetChunkLocalPosition(_chunkCount - 1, _elementCount);
		if (chunkObjectCount >= _objectCountPerChunk)
		{
			elementChunkIndex = AllocateChunk();
			elementLocalPosition = 0;
		}

		SStackIndex si = {};
		si.chunkIndex = elementChunkIndex;
		si.localPosition = elementLocalPosition;
		si.globalPosition = _elementCount++;

		return si;
	}

	template <typename TValue>
	types::u32 XDynamicArray<TValue>::AllocateChunk()
	{
		if (_chunkCount >= _allocatorDesc.maxChunkCount)
			return 0;

		const sCapabilities* caps = _context->GetSubsystem<cEngine>()->GetCapabilities();
		cMemoryAllocator* memoryAllocator = _context->GetMemoryAllocator();
		_chunkValues[_chunkCount] = (TValue*)memoryAllocator->Allocate(_allocatorDesc.chunkByteSize, caps->memoryAlignment);
		_chunkIndices[_chunkCount] = (SStackIndex*)memoryAllocator->Allocate(_objectCountPerChunk * sizeof(SStackIndex), caps->memoryAlignment);

		return _chunkCount++;
	}

	template <typename TValue>
	void XDynamicArray<TValue>::DeallocateChunk(types::u32 chunkIndex)
	{
		if (chunkIndex >= _chunkCount--)
			return;

		cMemoryAllocator* memoryAllocator = _context->GetMemoryAllocator();
		memoryAllocator->Deallocate(_chunkIndices[chunkIndex]);
		memoryAllocator->Deallocate(_chunkValues[chunkIndex]);
	}

	template <typename TValue>
	types::u32 XDynamicArray<TValue>::GetChunkIndex(types::u32 globalPosition) const
	{
		return globalPosition / _objectCountPerChunk;
	}

	template <typename TValue>
	types::u32 XDynamicArray<TValue>::GetChunkLocalPosition(types::u32 chunkIndex, types::u32 globalPosition) const
	{
		const types::u32 chunkIndexBoundary = chunkIndex * _objectCountPerChunk;
		const types::u32 localPosition = globalPosition - chunkIndexBoundary;

		return localPosition;
	}
}