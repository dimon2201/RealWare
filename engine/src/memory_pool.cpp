#include <iostream>
#include <cstdlib>
#include "memory_pool.hpp"

using namespace types;

namespace realware
{
    namespace utils
    {
        cMemoryPool::cMemoryPool(const usize byteSize, const usize allocs, const usize alignment)
        {
            if (alignment == 0)
                _memory = malloc(byteSize);
            else
                _memory = _aligned_malloc(byteSize, alignment);

            _byteSize = byteSize;
            _lastAddress = _memory;
            _maxAddress = (void*)(((usize)_memory) + byteSize);
            _allocs.reserve(allocs);
            _alignment = alignment;
        }

        cMemoryPool::~cMemoryPool()
        {
            if (_alignment == 0)
                free(_memory);
            else
                _aligned_free(_memory);

            _allocs.clear();
        }

        void* cMemoryPool::Allocate(const usize size)
        {
#ifdef DEBUG
            _bytesOccupied += size;
#endif
            for (auto& alloc : _allocs)
            {
                if (alloc.FreeFlag == 1 && alloc.AllocationByteSize >= size)
                {
                    alloc.FreeFlag = 255;
                    alloc.OccupiedByteSize = size;

                    return alloc.Address;
                }

                if ((usize)(alloc.AllocationByteSize - alloc.OccupiedByteSize) >= size)
                {
                    sMemoryPoolAllocation newAlloc;
                    newAlloc.FreeFlag = 0;
                    newAlloc.AllocationByteSize = alloc.AllocationByteSize - alloc.OccupiedByteSize;
                    newAlloc.OccupiedByteSize = size;
                    newAlloc.Address = (void*)((usize)alloc.Address + (usize)alloc.OccupiedByteSize);

                    alloc.AllocationByteSize = alloc.OccupiedByteSize;

                    _allocs.emplace_back(newAlloc);

                    return newAlloc.Address;
                }
            }

            if ((usize)_lastAddress + size >= (usize)_maxAddress)
            {
                std::cout << "Error: memory pool byte size '" << _byteSize << "' is not enough to allocate next '" << size << "' bytes!" << std::endl;
                
                return nullptr;
            }

            sMemoryPoolAllocation newAlloc;
            newAlloc.FreeFlag = 0;
            newAlloc.AllocationByteSize = size;
            newAlloc.OccupiedByteSize = size;
            newAlloc.Address = _lastAddress;
            _allocs.emplace_back(newAlloc);

            _lastAddress = (void*)((usize)_lastAddress + size);

            return newAlloc.Address;
        }

        bool cMemoryPool::Free(void* address)
        {
            for (auto& alloc : _allocs)
            {
                if (alloc.Address == address)
                {
#ifdef DEBUG
                    _bytesFreed += alloc.OccupiedByteSize;
                    _lastFreedBytes = alloc.OccupiedByteSize;
#endif
                    alloc.FreeFlag = 1;
                    alloc.OccupiedByteSize = 0;

                    return true;
                }
            }

            return false;
        }
    }
}