// dynamic_array.hpp

#pragma once

#include <type_traits>
#include <algorithm>
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

		SStackIndex NewIndex();
		types::u32 AllocateChunk();
		void DeallocateChunk(types::u32 chunkIndex);
		types::u32 GetChunkIndex(types::u32 globalPosition) const;
		types::u32 GetLocalPosition(types::u32 chunkIndex, types::u32 globalPosition) const;

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
		void Pop();
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
		SStackIndex si = NewIndex();
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
		SStackIndex si = NewIndex();
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
		const types::u32 localPosition = GetLocalPosition(chunkIndex, index);

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

		const types::u32 curChunkIndex = GetChunkIndex(index);
		const types::u32 curLocalPos = GetLocalPosition(curChunkIndex, index);
		const types::u32 lastChunkIndex = GetChunkIndex(_elementCount - 1);
		const types::u32 lastLocalPos = GetLocalPosition(lastChunkIndex, _elementCount - 1);
		if (index != _elementCount - 1)
		{
			_chunkValues[curChunkIndex][curLocalPos].~TValue();
			new (&_chunkValues[curChunkIndex][curLocalPos])
				TValue(std::move(_chunkValues[lastChunkIndex][lastLocalPos]));
			_chunkIndices[curChunkIndex][curLocalPos] = _chunkIndices[lastChunkIndex][lastLocalPos];
			_chunkValues[lastChunkIndex][lastLocalPos].~TValue();
		}
		else
		{
			_chunkValues[curChunkIndex][curLocalPos].~TValue();

		}
		const types::usize lastChunkObjectCount = _elementCount - lastChunkIndex * _objectCountPerChunk;
		if (lastChunkObjectCount == 1)
			DeallocateChunk(lastChunkIndex);

		_elementCount -= 1;
	}

	template <typename TValue>
	void XDynamicArray<TValue>::Pop()
	{
		if (_elementCount == 0)
			return;

		Erase(_elementCount - 1);
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
	SStackIndex XDynamicArray<TValue>::NewIndex()
	{
		types::u32 chunkIndex;
		types::u32 localPosition = 0;
		if (_chunkCount > 0)
		{
			const types::boolean bAllocateNew = _elementCount >= _chunkCount * _objectCountPerChunk;
			if (bAllocateNew == types::K_TRUE)
			{
				chunkIndex = AllocateChunk();
			}
			else
			{
				chunkIndex = GetChunkIndex(_elementCount);
				localPosition = GetLocalPosition(chunkIndex, _elementCount);
			}
		}
		else
		{
			chunkIndex = AllocateChunk();
		}

		SStackIndex si = {};
		si.chunkIndex = chunkIndex;
		si.localPosition = localPosition;
		si.globalPosition = _elementCount;

		_elementCount += 1;

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
		return globalPosition / (types::u32)_objectCountPerChunk;
	}

	template <typename TValue>
	types::u32 XDynamicArray<TValue>::GetLocalPosition(types::u32 chunkIndex, types::u32 globalPosition) const
	{
		return globalPosition - chunkIndex * _objectCountPerChunk;
	}
}