// context.hpp

#pragma once

#include <unordered_map>
#include "object.hpp"
#include "engine.hpp"
#include "capabilities.hpp"
#include "types.hpp"

namespace triton
{
	class cMemoryAllocator;
	class iBackend;

	class cContext
	{
		cMemoryAllocator* _allocator = nullptr;
		::std::unordered_map<ClassType, std::shared_ptr<iBackend>> _backends;
		::std::unordered_map<ClassType, iObject*> _subsystems;

	public:
		explicit cContext() = default;
		~cContext() = default;

		template <typename T, typename... Args>
		T* Create(Args&&... args);

		template <typename T, typename... Args>
		T* Create(T* ptr, types::u32 index, Args&&... args);

		template <typename T>
		void Destroy(T* object);

		cMemoryAllocator* CreateMemoryAllocator();
		void DestroyMemoryAllocator(cMemoryAllocator* allocator);

		template <typename T>
		void RegisterBackend(T* backend);

		void RegisterSubsystem(iObject* object);
		
		template <typename T>
		void ReleaseSubsystem();

		inline cMemoryAllocator* GetMemoryAllocator() const { return _allocator; }

		template <typename T>
		inline T* GetBackend() const;

		template <typename T>
		inline T* GetSubsystem() const;
	};
}

template <typename T, typename... Args>
T* triton::cContext::Create(Args&&... args)
{
	// TODO: rewrite object creation system completely
	// temporary solution

	const sCapabilities* caps = GetSubsystem<cEngine>()->GetCapabilities();
	cMemoryAllocator* memoryAllocator = GetMemoryAllocator();
	T* object = (T*)memoryAllocator->Allocate(sizeof(T), caps->memoryAlignment);
	
	new (object) T(std::forward<Args>(args)...);

	return object;

	/*const ClassType type = T::GetTypeStatic();
	const auto it = _factories.find(type);
	if (it != _factories.end())
		return ((cFactory<T>*)it->second)->Create(std::forward<Args>(args)...);
	else
		return nullptr;*/
}

template <typename T, typename... Args>
T* triton::cContext::Create(T* ptr, types::u32 index, Args&&... args)
{
	// TODO: rewrite object creation system completely
	// temporary solution

	new (&ptr[index]) T(std::forward<Args>(args)...);

	return &ptr[index];

	/*const ClassType type = T::GetTypeStatic();
	const auto it = _factories.find(type);
	if (it != _factories.end())
		return ((cFactory<T>*)it->second)->Create(ptr, index, std::forward<Args>(args)...);
	else
		return nullptr;*/
}

template <typename T>
void triton::cContext::Destroy(T* object)
{
	if (object == nullptr)
		return;

	object->~T();
	GetMemoryAllocator()->Deallocate(object);
}

template <typename T>
void triton::cContext::RegisterBackend(T* backend)
{
	// TODO: static_assert that T must inherit from iBackend
	const ClassType type = T::GetTypeStatic();
	const auto it = _backends.find(type);
	if (it == _backends.end())
		_backends.insert({ type, std::shared_ptr<iBackend>(backend) });
}

template <typename T>
void triton::cContext::ReleaseSubsystem()
{
	const ClassType type = T::GetTypeStatic();
	const auto it = _subsystems.find(type);
	if (it == _subsystems.end())
		delete it->second;
}

template <typename T>
T* triton::cContext::GetBackend() const
{
	const ClassType type = T::GetTypeStatic();
	const auto it = _backends.find(type);
	if (it != _backends.end())
		return (T*)it->second.get();
	else
		return nullptr;
}

template <typename T>
T* triton::cContext::GetSubsystem() const
{
	const ClassType type = T::GetTypeStatic();
	const auto it = _subsystems.find(type);
	if (it != _subsystems.end())
		return (T*)it->second;
	else
		return nullptr;
}