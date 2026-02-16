// context.hpp

#pragma once

#include <unordered_map>
#include "object.hpp"
#include "factory.hpp"
#include "types.hpp"

namespace triton
{
	class cMemoryAllocator;
	class iBackend;

	class cContext
	{
		cMemoryAllocator* _allocator = nullptr;
		::std::unordered_map<ClassType, std::shared_ptr<iBackend>> _backends;
		::std::unordered_map<ClassType, iObject*> _factories;
		::std::unordered_map<ClassType, iObject*> _subsystems;

	public:
		explicit cContext() = default;
		~cContext() = default;

		template <typename T, typename... Args>
		T* Create(Args&&... args);

		template <typename T, typename... Args>
		T* Create(types::u8* ptr, types::u32 index, Args&&... args);

		template <typename T>
		void Destroy(T* object);

		cMemoryAllocator* CreateMemoryAllocator();
		void DestroyMemoryAllocator(cMemoryAllocator* allocator);

		template <typename T>
		void RegisterFactory();

		template <typename T>
		void RegisterBackend(T* backend);

		void RegisterSubsystem(iObject* object);

		inline cMemoryAllocator* GetMemoryAllocator() const { return _allocator; }

		template <typename T>
		inline T* GetFactory() const;

		template <typename T>
		inline T* GetBackend() const;

		template <typename T>
		inline T* GetSubsystem() const;
	};
}

template <typename T, typename... Args>
T* triton::cContext::Create(Args&&... args)
{
	const ClassType type = T::GetTypeStatic();
	const auto it = _factories.find(type);
	if (it != _factories.end())
		return ((cFactory<T>*)it->second)->Create(std::forward<Args>(args)...);
	else
		return nullptr;
}

template <typename T, typename... Args>
T* triton::cContext::Create(types::u8* ptr, types::u32 index, Args&&... args)
{
	const ClassType type = T::GetTypeStatic();
	const auto it = _factories.find(type);
	if (it != _factories.end())
		return ((cFactory<T>*)it->second)->Create(ptr, index, std::forward<Args>(args)...);
	else
		return nullptr;
}

template <typename T>
void triton::cContext::Destroy(T* object)
{
	const ClassType type = T::GetTypeStatic();
	const auto it = _factories.find(type);
	if (it != _factories.end())
		((cFactory<T>*)it->second)->Destroy(object);
}

template <typename T>
void triton::cContext::RegisterFactory()
{
	// TODO: static_assert that T must inherit from iObject
	const ClassType type = T::GetTypeStatic();
	const auto it = _factories.find(type);
	if (it == _factories.end())
		_factories.insert({type, new cFactory<T>(this)});
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
T* triton::cContext::GetFactory() const
{
	const ClassType type = T::GetTypeStatic();
	const auto it = _factories.find(type);
	if (it != _factories.end())
		return (T*)it->second;
	else
		return nullptr;
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