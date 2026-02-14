// context.cpp

#include "context.hpp"
#include "memory_pool.hpp"

void triton::cContext::CreateMemoryAllocator()
{
	_allocator = new cMemoryAllocator();
	_allocator->SetBins(65536);
}

void triton::cContext::RegisterSubsystem(iObject* object)
{
	// TODO: static_assert that T must inherit from iObject
	ClassType type = object->GetType();
	const auto it = _subsystems.find(type);
	if (it == _subsystems.end())
		_subsystems.insert({ type, object });
}