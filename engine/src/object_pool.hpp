// object_pool.hpp

#pragma once

#include <vector>
#include <queue>
#include <optional>
#include <algorithm>
#include <functional>
#include "object.hpp"
#include "handle.hpp"
#include "object_allocator.hpp"
#include "synchronization.hpp"
#include "buffer_view.hpp"
#include "gpu_buffer_types.hpp"
#include "graphics_backend.hpp"
#include "context.hpp"
#include "engine.hpp"
#include "pool.hpp"
#include "types.hpp"

namespace triton
{
    template <typename THandle>
    struct SObjectFrame
    {
        types::usize count = 0;
        THandle begin;
    };

    template <typename TObject>
    class CObjectPool : public CPool
    {
    public:
        static constexpr types::usize   cDefaultReservationSize = 1024;

    protected:
        cContext*                       _context = nullptr;
        SSlot*                          _slots = nullptr;
        TObject*                        _objects = nullptr;
        typename TObject::TGPULayout*   _staging = nullptr;
        types::boolean				    _bKeepStagingBuffer = types::K_FALSE;
        types::usize				    _reservedCount = 0;
        types::usize					_reservationSize = cDefaultReservationSize;
        types::usize				    _lastObjectCursor = 0;
        std::queue<types::usize>	    _freeSlots = {};
        types::boolean				    _bDirtyBit = types::K_FALSE;
        CGPUBufferResource              _gpuBuffer;
        types::s32                      _gpuBufferSlot = -1;
        EGPUBufferType                  _gpuBufferType = EGPUBufferType::Unknown;

    public:
        explicit CObjectPool(
            cContext* context,
            types::boolean bKeepCpuCopy,
            types::usize reservationSize = cDefaultReservationSize,
            types::s32 gpuBufferSlot = -1,
            EGPUBufferType gpuBufferType = EGPUBufferType::Unknown
        );

        virtual ~CObjectPool();

        void Reserve(types::usize reserveCount);

        template <typename... Args>
        std::optional<typename TObject::THandle> Create(Args&&... args);

        std::optional<SObjectFrame<typename TObject::THandle>> CreateFrame(types::usize count);

        void Destroy(const typename TObject::THandle& handle);

        void DestroyFrame(const SObjectFrame<typename TObject::THandle>& frame);

        std::optional<std::reference_wrapper<TObject>> Get(const typename TObject::THandle& handle);

        void WriteToStaging(types::usize bufferIndex, const TObject& object);

        types::usize GetPackedIndex(const typename TObject::THandle& handle);

        types::usize GetPackedIndex(types::s32 freeIndex);

        std::optional<typename TObject::THandle> GetHandle(
            const typename TObject::THandle& baseHandle,
            const types::usize offset
        );

        void Upload();

        inline SBufferView<TObject> GetData() const
        {
            return SBufferView<TObject>(&_objects[0], sizeof(TObject) * _lastObjectCursor);
        }

        inline CGPUBufferResource GetGPUBuffer() const
        {
            return _gpuBuffer;
        }

        virtual typename TObject::TGPULayout ConvertToGpuLayout(const TObject& object) = 0;

        virtual void Update() = 0;

    private:
        template <typename TBufferObject>
        void AllocatePodBuffer(TBufferObject*& buffer, types::usize byteSize);

        template <typename TBufferObject>
        void ReallocatePodBuffer(
            TBufferObject*& buffer,
            types::usize prevMaxCount,
            types::usize curMaxCount
        );

        void AllocateObjectBuffer(types::usize byteSize);

        void ReallocateObjectBuffer(types::usize prevMaxCount, types::usize curMaxCount);

        void DestroyObjectBuffer();

        void AllocateGpuBuffer(
            CGPUBufferResource& buffer,
            EGPUBufferType type,
            types::usize byteSize,
            types::s32 slot
        );

        void DestroyGpuBuffer();

        void OccupySlot(types::usize objectIndex, types::usize generation);

        template <typename... Args>
        void ConstructObject(types::usize objectIndex, Args&&... args);
    };

    template <typename TObject>
    CObjectPool<TObject>::CObjectPool(
        cContext* context,
        types::boolean bKeepStagingBuffer,
        types::usize reservationSize,
        types::s32 gpuBufferSlot,
        EGPUBufferType gpuBufferType
    ) : CPool(context),
        _context(context),
        _bKeepStagingBuffer(bKeepStagingBuffer),
        _reservationSize(reservationSize),
        _gpuBufferSlot(gpuBufferSlot),
        _gpuBufferType(gpuBufferType)
    {
        if (_reservationSize == 0)
            _reservationSize = cDefaultReservationSize;

        Reserve(reservationSize);
    }
    
    template <typename TObject>
    CObjectPool<TObject>::~CObjectPool()
    {
        DestroyGpuBuffer();
        if (_staging)
            CObjectAllocator::Deallocate(_staging);
        if (_objects)
            DestroyObjectBuffer();
        if (_slots)
            CObjectAllocator::Deallocate(_slots);
    }

    template <typename TObject>
    void CObjectPool<TObject>::Reserve(types::usize reserveCount)
    {
        _bDirtyBit = types::K_TRUE;

        const types::usize prevReservedCount = _reservedCount;
        _reservedCount = reserveCount;

        if (_slots)
            ReallocatePodBuffer<SSlot>(_slots, prevReservedCount, reserveCount);
        else
            AllocatePodBuffer(_slots, reserveCount * sizeof(SSlot));

        if (_objects)
            ReallocateObjectBuffer(prevReservedCount, reserveCount);
        else
            AllocateObjectBuffer(reserveCount * sizeof(TObject));
        
        if (_bKeepStagingBuffer == types::K_TRUE)
        {
            if (_staging)
                ReallocatePodBuffer<typename TObject::TGPULayout>(_staging, prevReservedCount, reserveCount);
            else
                AllocatePodBuffer(_staging, reserveCount * sizeof(typename TObject::TGPULayout));
        }
        else
        {
            _staging = nullptr;
        }

        if (_gpuBuffer.GetBufferType() != EGPUBufferType::Unknown && _gpuBufferSlot > -1)
        {
            DestroyGpuBuffer();
            AllocateGpuBuffer(
                _gpuBuffer,
                _gpuBufferType,
                reserveCount * sizeof(typename TObject::TGPULayout),
                _gpuBufferSlot
            );
            Upload();
        }
        else if (_gpuBufferSlot > -1)
        {
            AllocateGpuBuffer(
                _gpuBuffer,
                _gpuBufferType,
                reserveCount * sizeof(typename TObject::TGPULayout),
                _gpuBufferSlot
            );
        }
    }

    template <typename TObject>
    template <typename... Args>
    std::optional<typename TObject::THandle> CObjectPool<TObject>::Create(Args&&... args)
    {
        types::usize objectIndex = 0;
        if (_freeSlots.empty())
        {
            objectIndex = _lastObjectCursor++;
            if (objectIndex >= _reservedCount)
                Reserve(_reservedCount + _reservationSize);
        }
        else
        {
            objectIndex = _freeSlots.front();
            _freeSlots.pop();
        }

        OccupySlot(objectIndex, _slots[objectIndex].generation + 1);

        new (&_objects[objectIndex]) TObject(_context, objectIndex, std::forward<Args>(args)...);

        typename TObject::THandle handle;
        handle.index = objectIndex;
        handle.generation = _slots[objectIndex].generation;

        return handle;
    }

    template <typename TObject>
    std::optional<SObjectFrame<typename TObject::THandle>> CObjectPool<TObject>::CreateFrame(types::usize count)
    {
        for (types::usize i = 0; i < _reservedCount; i++)
        {
            const types::usize requiredCount = i + count;

            types::boolean bFree = types::K_TRUE;
            for (types::usize j = 0; j < count; j++)
            {
                if (i + j >= _reservedCount)
                    break;

                if (_slots[i + j].alive == types::K_TRUE)
                {
                    bFree = types::K_FALSE;
                    break;
                }
            }

            if (bFree == types::K_TRUE)
            {
                if (requiredCount > _reservedCount)
                {
                    const types::usize newReservedCount =
                        ((requiredCount + _reservationSize - 1) / _reservationSize) * _reservationSize;
                    Reserve(newReservedCount);
                }

                while (!_freeSlots.empty())
                    _freeSlots.pop();

                for (types::usize j = 0; j < count; j++)
                {
                    OccupySlot(i + j, _slots[i + j].generation + 1);

                    new (&_objects[i + j]) TObject(_context, i + j);
                }

                for (types::usize j = 0; j < _lastObjectCursor; j++)
                    if (_slots[j].alive == types::K_FALSE)
                        _freeSlots.push(j);

                if (requiredCount > _lastObjectCursor)
                    _lastObjectCursor = requiredCount;

                typename TObject::THandle begin;
                begin.index = i;
                begin.generation = _slots[i].generation;

                SObjectFrame<typename TObject::THandle> frame;
                frame.count = count;
                frame.begin = begin;

                return frame;
            }
        }

        return std::nullopt;
    }

    template <typename TObject>
    void CObjectPool<TObject>::Destroy(const typename TObject::THandle& handle)
    {
        if (handle.generation != _slots[handle.index].generation ||
            _slots[handle.index].alive == types::K_FALSE)
            return;

        _objects[handle.index].~TObject();
        _slots[handle.index].alive = types::K_FALSE;
        _freeSlots.push(handle.index);
    }

    template <typename TObject>
    void CObjectPool<TObject>::DestroyFrame(const SObjectFrame<typename TObject::THandle>& frame)
    {
        for (types::usize i = 0; i < frame.count; i++)
        {
            _objects[frame.begin.index + i].~TObject();
            _slots[frame.begin.index + i].alive = types::K_FALSE;
            _freeSlots.push(frame.begin.index + i);
        }
    }

    template <typename TObject>
    std::optional<std::reference_wrapper<TObject>> CObjectPool<TObject>::Get(const typename TObject::THandle& handle)
    {
        if (handle.generation != _slots[handle.index].generation ||
            _slots[handle.index].alive == types::K_FALSE)
            return std::nullopt;

        return _objects[handle.index];
    }

    template <typename TObject>
    void CObjectPool<TObject>::WriteToStaging(types::usize bufferIndex, const TObject& object)
    {
        _staging[bufferIndex] = ConvertToGpuLayout(object);
        _bDirtyBit = types::K_TRUE;
    }

    template <typename TObject>
    types::usize CObjectPool<TObject>::GetPackedIndex(const typename TObject::THandle& handle)
    {
        if (handle.generation != _slots[handle.index].generation ||
            _slots[handle.index].alive == types::K_FALSE)
            return 0;

        types::usize packedIndex = 0;
        for (types::usize i = 0; i < handle.index; i++)
            if (_slots[i].alive == types::K_TRUE)
                ++packedIndex;

        return packedIndex;
    }

    template <typename TObject>
    types::usize CObjectPool<TObject>::GetPackedIndex(types::s32 freeIndex)
    {
        types::usize packedIndex = 0;
        for (types::usize i = 0; i < freeIndex; i++)
            if (_slots[i].alive == types::K_TRUE)
                ++packedIndex;

        return packedIndex;
    }

    template <typename TObject>
    std::optional<typename TObject::THandle> CObjectPool<TObject>::GetHandle(
        const typename TObject::THandle& baseHandle,
        const types::usize offset
    )
    {
        if (baseHandle.generation != _slots[baseHandle.index].generation ||
            _slots[baseHandle.index].alive == types::K_FALSE)
            return std::nullopt;

        typename TObject::THandle offsetHandle;
        offsetHandle.index = baseHandle.index + offset;
        offsetHandle.generation = _slots[offsetHandle.index].generation;

        return offsetHandle;
    }

    template <typename TObject>
    void CObjectPool<TObject>::Upload()
    {
        if (_gpuBufferSlot == -1 || _gpuBufferType == EGPUBufferType::Unknown)
            return;

        if (_bDirtyBit == types::K_TRUE)
        {
            types::usize packedSize = 0;
            for (types::usize i = 0; i < _lastObjectCursor; i++)
                if (_slots[i].alive == types::K_TRUE)
                    ++packedSize;

            _context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
                ERenderCommand::WRITE_BUFFER,
                (types::cpuword)&_gpuBuffer,
                0,
                packedSize * sizeof(typename TObject::TGPULayout),
                (types::cpuword)&_staging[0]
            ));
            _context->GetSubsystem<CEngine>()->GetSynchronization()->WaitForRenderCommandResult<void*>();

            _bDirtyBit = types::K_FALSE;
        }
    }

    template <typename TObject>
    template <typename TBufferObject>
    void CObjectPool<TObject>::AllocatePodBuffer(TBufferObject*& buffer, types::usize byteSize)
    {
        buffer = (TBufferObject*)CObjectAllocator::Allocate(byteSize, 64);
        const types::usize count = byteSize / sizeof(TBufferObject);
        for (types::usize i = 0; i < count; i++)
            new (&buffer[i]) TBufferObject();
    }

    template <typename TObject>
    template <typename TBufferObject>
    void CObjectPool<TObject>::ReallocatePodBuffer(
        TBufferObject*& buffer,
        types::usize prevMaxCount,
        types::usize curMaxCount
    )
    {
        const types::usize objectByteSize = sizeof(TBufferObject);
        TBufferObject* temp = (TBufferObject*)CObjectAllocator::Allocate(curMaxCount * objectByteSize, 64);
        const types::usize copySize = std::min(prevMaxCount, curMaxCount);
        for (types::usize i = 0; i < curMaxCount; i++)
            new (&temp[0]) TBufferObject();
        for (types::usize i = 0; i < copySize; i++)
            temp[i] = buffer[i];
        CObjectAllocator::Deallocate(buffer);
        buffer = temp;
    }

    template <typename TObject>
    void CObjectPool<TObject>::AllocateObjectBuffer(types::usize byteSize)
    {
        _objects = (TObject*)CObjectAllocator::Allocate(byteSize, 64);
    }

    template <typename TObject>
    void CObjectPool<TObject>::ReallocateObjectBuffer(types::usize prevMaxCount, types::usize curMaxCount)
    {
        const types::usize objectByteSize = sizeof(TObject);
        TObject* temp = (TObject*)CObjectAllocator::Allocate(curMaxCount * objectByteSize, 64);
        const types::usize copyElementSize = std::min(prevMaxCount, curMaxCount);
        for (types::usize slotIndex = 0; slotIndex < copyElementSize; slotIndex++)
        {
            if (_slots[slotIndex].alive == types::K_FALSE)
                continue;
            CObjectAllocator::Create(&temp[0], slotIndex, std::move(_objects[slotIndex]));
            _objects[slotIndex].~TObject();
        }
        CObjectAllocator::Deallocate(_objects);
        _objects = temp;
    }

    template <typename TObject>
    void CObjectPool<TObject>::DestroyObjectBuffer()
    {
        for (types::usize i = 0; i < _lastObjectCursor; i++)
            if (_slots[i].alive == types::K_TRUE)
                _objects[i].~TObject();
        CObjectAllocator::Deallocate(_objects);

        _lastObjectCursor = 0;
    }

    template <typename TObject>
    void CObjectPool<TObject>::AllocateGpuBuffer(
        CGPUBufferResource& buffer,
        EGPUBufferType type,
        types::usize byteSize,
        types::s32 slot
    )
    {
        _context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
            ERenderCommand::CREATE_BUFFER,
            (types::cpuword)type,
            (types::cpuword)nullptr,
            byteSize,
            slot
        ));
        buffer = _context->GetSubsystem<CEngine>()->
            GetSynchronization()->
            WaitForRenderCommandResult<CGPUBufferResource>();
    }


    template <typename TObject>
    void CObjectPool<TObject>::DestroyGpuBuffer()
    {
        if (_gpuBufferSlot < 0 || _gpuBufferType == EGPUBufferType::Unknown)
            return;

        _context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
            ERenderCommand::DESTROY_BUFFER,
            (types::cpuword)&_gpuBuffer
        ));
        _context->GetSubsystem<CEngine>()->GetSynchronization()->WaitForRenderCommandResult<void*>();
    }

    template <typename TObject>
    void CObjectPool<TObject>::OccupySlot(types::usize objectIndex, types::usize generation)
    {
        _slots[objectIndex].generation = generation;
        _slots[objectIndex].alive = types::K_TRUE;
    }

    template <typename TObject>
    template <typename... Args>
    void CObjectPool<TObject>::ConstructObject(types::usize objectIndex, Args&&... args)
    {
        CObjectAllocator::Create<TObject>(&_objects[0], objectIndex, std::forward<Args>(args)...);
    }
}