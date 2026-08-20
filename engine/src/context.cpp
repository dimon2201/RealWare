// context.cpp

#include "context.hpp"
#include "memory_pool.hpp"
#include "subsystem.hpp"
#include "pool.hpp"

triton::cMemoryAllocator* triton::cContext::CreateMemoryAllocator()
{
	_allocator = new cMemoryAllocator();
	_allocator->SetBins(65536);

	return _allocator;
}

void triton::cContext::DestroyMemoryAllocator(cMemoryAllocator* allocator)
{
	delete _allocator;
}

void triton::cContext::RegisterSubsystem(CSubsystem* subsystem)
{
	// TODO: static_assert that T must inherit from iObject
	ClassType type = subsystem->GetType();
	const auto it = _subsystems.find(type);
	if (it == _subsystems.end())
		_subsystems.insert({ type, subsystem });
}

void triton::cContext::RegisterPool(CPool* pool)
{
	ClassType type = pool->GetType();
	const auto it = _pools.find(type);
	if (it == _pools.end())
		_pools.insert({ type, pool });
}