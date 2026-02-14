// buffer.cpp

#pragma once

#include "buffer.hpp"

triton::cDataBuffer::cDataBuffer(cContext* context) : iObject(context) {}

triton::cDataBuffer::~cDataBuffer()
{
}

void triton::cDataBuffer::Create(void* data, types::usize byteSize)
{
}