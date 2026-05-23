// buffer.cpp

#pragma once

#include "buffer.hpp"
#include "capabilities.hpp"
#include "context.hpp"
#include "engine.hpp"
#include "application.hpp"

using namespace types;

triton::XDataBuffer::XDataBuffer(cContext* context, usize byteSize) : iObject(context)
{
	const sCapabilities* caps = _context->GetSubsystem<cEngine>()->GetApplication()->GetCapabilities();
	auto memoryAllocator = _context->GetMemoryAllocator();

	_byteSize = byteSize;
	_data = (u8*)memoryAllocator->Allocate(_byteSize, caps->memoryAlignment);
	memset(_data, 0, _byteSize);
}

triton::XDataBuffer::~XDataBuffer()
{
	if (_data != nullptr)
	{
		const sCapabilities* caps = _context->GetSubsystem<cEngine>()->GetApplication()->GetCapabilities();
		auto memoryAllocator = _context->GetMemoryAllocator();

		memoryAllocator->Deallocate(_data);
	}
}

void triton::XDataBuffer::Write(const u8* data, usize byteSize, usize byteOffset)
{
	if (data == nullptr || byteSize >= _byteSize || byteOffset >= _byteSize)
	{
		return;
	}

	memcpy(&_data[byteOffset], data, byteSize);
}