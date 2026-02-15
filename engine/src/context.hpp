// context.hpp

#pragma once

#include <unordered_map>
#include "object.hpp"
#include "factory.hpp"
#include "platform.hpp"
#include "types.hpp"

namespace triton
{
	class cMemoryAllocator;
	class iBackend;

	class cContext
	{
	public:
		explicit cContext() = default;
		~cContext() = default;

		template <typename T, typename... Args>
		T* Create(Args&&... args);

		template <typename T, typename... Args>
		T* Create(types::u8* ptr, types::u32 index, Args&&... args);

		template <typename T>
		void Destroy(T* object);

		cPlatform* CreatePlatform(cPlatform::eInputBackend inputBackend, cPlatform::eGraphicsBackend graphicsBackend);
		cMemoryAllocator* CreateMemoryAllocator();
		void DestroyPlatform(cPlatform* platform);
		void DestroyAllocator(cMemoryAllocator* allocator);

		template <typename T>
		void RegisterFactory();

		void RegisterSubsystem(iObject* object);

		inline cPlatform* GetPlatform() const { return _platform; }
		inline cMemoryAllocator* GetMemoryAllocator() const { return _allocator; }

		template <typename T>
		inline T* GetBackend() const;

		template <typename T>
		inline T* GetFactory() const;

		template <typename T>
		inline T* GetSubsystem() const;

	private:
		cPlatform* _platform = nullptr;
		cMemoryAllocator* _allocator = nullptr;
		::std::unordered_map<ClassType, iObject*> _factories;
		::std::unordered_map<ClassType, iObject*> _subsystems;
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
T* triton::cContext::GetBackend() const
{
	return _platform->GetBackend<T>();
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
T* triton::cContext::GetSubsystem() const
{
	const ClassType type = T::GetTypeStatic();
	const auto it = _subsystems.find(type);
	if (it != _subsystems.end())
		return (T*)it->second;
	else
		return nullptr;
}