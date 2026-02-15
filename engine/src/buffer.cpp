// buffer.cpp

#pragma once

#include "buffer.hpp"
#include "capabilities.hpp"
#include "context.hpp"
#include "engine.hpp"
#include "application.hpp"

using namespace types;

triton::cDataBuffer::cDataBuffer(cContext* context, usize byteSize) : iObject(context)
{
	const sCapabilities* caps = _context->GetSubsystem<cEngine>()->GetApplication()->GetCapabilities();
	auto memoryAllocator = _context->GetMemoryAllocator();

	_byteSize = byteSize;
	_data = (u8*)memoryAllocator->Allocate(_byteSize, caps->memoryAlignment);
	memset(_data, 0, _byteSize);
}

triton::cDataBuffer::~cDataBuffer()
{
	if (_data != nullptr)
	{
		const sCapabilities* caps = _context->GetSubsystem<cEngine>()->GetApplication()->GetCapabilities();
		auto memoryAllocator = _context->GetMemoryAllocator();

		memoryAllocator->Deallocate(_data);
	}
}