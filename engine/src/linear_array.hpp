// linear_array.hpp

#pragma once

#include <type_traits>
#include "object.hpp"
#include "context.hpp"
#include "memory_pool.hpp"
#include "stack_value.hpp"
#include "buffer_view.hpp"
#include "types.hpp"

namespace triton
{
	template <typename TValue>
	class XLinearArray : public iObject
	{
		TRITON_OBJECT(XLinearArray)

		types::usize _elementCount = 0;
		TValue* _data = nullptr;

	public:
		explicit XLinearArray(cContext* context, types::usize maxByteSize);
		virtual ~XLinearArray() override final;

		template<typename... Args>
		TValue* Push(Args&&... args);
		TValue* Push(TValue&& value);
		template<typename... Args>
		TValue* Recreate(types::usize index, Args&&... args);
		TValue* Recreate(types::usize index, TValue&& value);
		TValue* At(types::u32 index) const;
		TValue* Top() const;
		void Erase(types::u32 index);
		void Pop();
		void Clear();
		types::boolean IsEmpty();

		inline types::usize GetSize() const
		{
			return _elementCount;
		}

		inline SBufferView GetData() const
		{
			return SBufferView((void*)_data, _elementCount * sizeof(XLinearArray));
		}
	};
}

template <typename TValue>
triton::XLinearArray<TValue>::XLinearArray(cContext* context, types::usize maxByteSize) : iObject(context)
{
	const sCapabilities* caps = _context->GetSubsystem<cEngine>()->GetCapabilities();
	cMemoryAllocator* memoryAllocator = _context->GetMemoryAllocator();
	_data = (TValue*)memoryAllocator->Allocate(maxByteSize, caps->memoryAlignment);
}

template <typename TValue>
triton::XLinearArray<TValue>::~XLinearArray()
{
	for (types::usize i = 0; i < _elementCount; i++)
		Pop();

	cMemoryAllocator* memoryAllocator = _context->GetMemoryAllocator();
	memoryAllocator->Deallocate(_data);
}

template <typename TValue>
template <typename... Args>
TValue* triton::XLinearArray<TValue>::Push(Args&&... args)
{
	TValue* object = _context->Create<TValue>((types::u8*)_data, _elementCount, std::forward<Args>(args)...);
	_elementCount += 1;

	return object;
}

template <typename TValue>
TValue* triton::XLinearArray<TValue>::Push(TValue&& value)
{
	_data[_elementCount] = std::move(value);
	TValue* object = &_data[_elementCount];
	_elementCount += 1;

	return object;
}

template <typename TValue>
template <typename... Args>
TValue* triton::XLinearArray<TValue>::Recreate(types::usize index, Args&&... args)
{
	if (_elementCount == 0 || index >= _elementCount)
		return nullptr;

	_data[index].~TValue();

	TValue* object = _context->Create<TValue>((types::u8*)_data, index, std::forward<Args>(args)...);

	return object;
}

template <typename TValue>
TValue* triton::XLinearArray<TValue>::Recreate(types::usize index, TValue&& value)
{
	if (_elementCount == 0 || index >= _elementCount)
		return nullptr;

	_data[index].~TValue();
	_data[index] = std::move(value);

	TValue* object = &_data[index];

	return object;
}

template <typename TValue>
TValue* triton::XLinearArray<TValue>::At(types::u32 index) const
{
	if (_dataCount == 0 || index >= _elementCount)
		return nullptr;

	return &_data[index];
}

template <typename TValue>
TValue* triton::XLinearArray<TValue>::Top() const
{
	if (_elementCount == 0)
		return nullptr;

	return At(_elementCount - 1);
}

template <typename TValue>
void triton::XLinearArray<TValue>::Erase(types::u32 index)
{
	if (_elementCount == 0 || index >= _elementCount)
		return;

	_data[index].~TValue();
	_data[index] = _data[_elementCount - 1];
	_elementCount -= 1;
}

template <typename TValue>
void triton::XLinearArray<TValue>::Pop()
{
	Erase(_elementCount - 1);
}

template <typename TValue>
void triton::XLinearArray<TValue>::Clear()
{
	types::usize size = GetSize();
	for (types::usize i = 0; i < size; i++)
		Pop();
}

template <typename TValue>
types::boolean triton::XLinearArray<TValue>::IsEmpty()
{
	return _elementCount == 0 ? types::K_TRUE : types::K_FALSE;
}