// context.cpp

#include "context.hpp"
#include "memory_pool.hpp"

triton::cPlatform* triton::cContext::CreatePlatform(cPlatform::eInputBackend inputBackend, cPlatform::eGraphicsBackend graphicsBackend)
{
	_platform = new cPlatform(this, inputBackend, graphicsBackend);

	return _platform;
}

triton::cMemoryAllocator* triton::cContext::CreateMemoryAllocator()
{
	_allocator = new cMemoryAllocator();
	_allocator->SetBins(65536);

	return _allocator;
}

void triton::cContext::DestroyPlatform(cPlatform* platform)
{
	delete _platform;
}

void triton::cContext::DestroyAllocator(cMemoryAllocator* allocator)
{
	delete _allocator;
}

void triton::cContext::RegisterSubsystem(iObject* object)
{
	// TODO: static_assert that T must inherit from iObject
	ClassType type = object->GetType();
	const auto it = _subsystems.find(type);
	if (it == _subsystems.end())
		_subsystems.insert({ type, object });
}