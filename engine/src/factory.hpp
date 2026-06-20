// factory.hpp

#pragma once

#include <string>
#include "capabilities.hpp"
#include "memory_pool.hpp"
#include "log.hpp"
#include "object.hpp"
#include "hash_table.hpp"
#include "engine.hpp"
#include "types.hpp"

namespace triton
{
	class cContext;

	template <typename T>
	class cFactory : public iObject
	{
		TRITON_OBJECT(cFactory)

		types::usize _counter = 0;

		types::boolean AssertCounter();
		template <typename... Args>
		T* New(types::u8* data, types::u32 index, Args&&... args);

	public:
		explicit cFactory(cContext* context);
		virtual ~cFactory() override final = default;

		template <typename... Args>
		T* Create(Args&&... args);
		template <typename... Args>
		T* Create(types::u8* data, types::u32 index, Args&&... args);
		void Destroy(T* object);
	};
}

template <typename T>
triton::cFactory<T>::cFactory(cContext* context) : iObject(context) {}

template <typename T>
template <typename... Args>
T* triton::cFactory<T>::Create(Args&&... args)
{
	AssertCounter();

	return New(nullptr, 0, std::forward<Args>(args)...);
}

template <typename T>
template <typename... Args>
T* triton::cFactory<T>::Create(types::u8* ptr, types::u32 index, Args&&... args)
{
	AssertCounter();

	return New(ptr, index, std::forward<Args>(args)...);
}

template <typename T>
void triton::cFactory<T>::Destroy(T* object)
{
	if (object == nullptr)
		return;

	object->~T();

	if (object->_allocatedUsingMemAllocator == types::K_TRUE)
	{
		// TODO: must rewrite object creation system completely
		// TEMPORARY STUB TO MAKE PROJECT COMPILE
		//cMemoryAllocator* memoryAllocator = _context->GetMemoryAllocator();
		//memoryAllocator->Deallocate(object);
	}
}

template <typename T>
types::boolean triton::cFactory<T>::AssertCounter()
{
	if (_counter >= types::K_USIZE_MAX)
	{
		Print("Error: can't create object of type '" + T::GetTypeStatic() + "'!");

		return types::K_TRUE;
	}

	return types::K_FALSE;
}

template <typename T>
template <typename... Args>
T* triton::cFactory<T>::New(types::u8* data, types::u32 index, Args&&... args)
{
	// TODO: must rewrite object creation system completely
	// TEMPORARY STUB TO MAKE PROJECT COMPILE
	/*T* object = nullptr;

	if (data == nullptr)
	{
		const sCapabilities* caps = _context->GetSubsystem<cEngine>()->GetCapabilities();
		cMemoryAllocator* memoryAllocator = _context->GetMemoryAllocator();
		object = (T*)memoryAllocator->Allocate(sizeof(T), caps->memoryAlignment);
		object->_allocatedUsingMemAllocator = types::K_TRUE;
	}
	else
	{
		object = &(((T*)data)[index]);
		object->_allocatedUsingMemAllocator = types::K_FALSE;
	}

	new (object) T(std::forward<Args>(args)...);

	object->_id = cIdentifier::Generate(T::GetTypeStatic());

	return object;*/
	return nullptr;
}