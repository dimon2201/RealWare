// object_allocator.hpp

#pragma once

#include "memory_pool.hpp"
#include "types.hpp"

namespace triton
{
	class CObjectAllocator final
	{
		inline static cMemoryAllocator* _allocator = nullptr;

	public:
		static void Initialize(cMemoryAllocator* allocator)
		{
			_allocator = allocator;
		}

		static void* Allocate(types::usize byteSize, types::usize alignmentByteSize)
		{
			if (!_allocator)
				return nullptr;

			return _allocator->Allocate(byteSize, alignmentByteSize);
		}

		static void Deallocate(void* ptr)
		{
			if (!_allocator)
				return;

			return _allocator->Deallocate(ptr);
		}

		template <typename T, typename... Args>
		static T* Create(types::usize alignmentByteSize, Args&&... args)
		{
			if (!_allocator)
				return nullptr;

			T* object = (T*)_allocator->Allocate(sizeof(T), alignmentByteSize);
			new (object) T(std::forward<Args>(args)...);

			return object;
		}

		template <typename T, typename... Args>
		static T* Create(T* ptr, types::u32 index, Args&&... args)
		{
			if (!_allocator)
				return nullptr;

			new (&ptr[index]) T(std::forward<Args>(args)...);

			return &ptr[index];
		}

		template <typename T>
		static T* Create(T* ptr, types::u32 index, T&& obj)
		{
			if (!_allocator)
				return nullptr;

			new (&ptr[index]) T(std::move(obj));

			return &ptr[index];
		}

		template <typename T>
		static void Destroy(T* object)
		{
			if (!_allocator || !object)
				return;

			object->~T();
			_allocator->Deallocate(object);
		}
	};
}