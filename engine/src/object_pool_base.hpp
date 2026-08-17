// object_pool_base.hpp

#pragma once

#include <vector>
#include <queue>
#include <optional>
#include <algorithm>
#include <functional>
#include "object.hpp"
#include "handle.hpp"
#include "object_allocator.hpp"
#include "graphics_resource_backend.hpp"
#include "material_data.hpp"
#include "synchronization.hpp"
#include "buffer_view.hpp"
#include "types.hpp"

namespace triton
{
    class cContext;

    template <typename THandle>
    struct SObjectFrame
    {
        types::usize count = 0;
        typename THandle begin;
    };

    template <typename TObject>
    class XObjectPoolBase : public iObject
    {
        TRITON_OBJECT(XObjectPoolBase)

    protected:
        SSlot*                      _slots = nullptr;
        TObject*                    _objects = nullptr;
        TObject::TGPULayout*        _staging = nullptr;
        types::boolean				_bKeepStagingBuffer = types::True;
        types::usize				_reservedCount = 0;
        types::usize				_lastObjectCursor = 0;
        std::queue<types::usize>	_freeSlots = {};
        types::boolean				_bDirtyBit = types::False;
        cBuffer*                    _gpuBuffer = nullptr;
        types::s32                  _gpuBufferSlot = -1;
        cBuffer::eType              _gpuBufferType = cBuffer::eType::NONE;

    public:
        explicit XObjectPoolBase(
            cContext* context,
            types::boolean bKeepCpuCopy,
            types::s32 gpuBufferSlot = -1,
            cBuffer::eType gpuBufferType = cBuffer::eType::NONE
        );

        ~XObjectPoolBase() override;

        void Allocate(types::usize maxCount);

        template <typename... Args>
        std::optional<typename TObject::THandle> Create(Args&&... args);

        std::optional<SObjectFrame<typename TObject::THandle>> Create(types::usize count);

        void Destroy(const TObject::THandle& handle);

        void Destroy(const SObjectFrame<typename TObject::THandle>& frame);

        std::optional<std::reference_wrapper<TObject>> Get(const TObject::THandle& handle);

        void WriteToStaging(types::usize bufferIndex, const TObject& object);

        types::usize GetPackedIndex(const TObject::THandle& handle);

        std::optional<typename TObject::THandle> GetHandle(
            const TObject::THandle& baseHandle,
            const types::usize offset
        );

        void Upload();

        inline SBufferView<TObject> GetData() const
        {
            return SBufferView<TObject>(&_objects[0], sizeof(TObject) * _lastObjectCursor);
        }

        inline cBuffer* GetGPUBuffer() const
        {
            return _gpuBuffer;
        }

        virtual TObject::TGPULayout ConvertToGpuLayout(const TObject& object) = 0;

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

        void ReallocateObjectBuffer(types::usize prevMaxCount, types::usize curMaxCount);

        void OccupySlot(types::usize objectIndex, types::usize generation);
        
        template <typename... Args>
        void ConstructObject(types::usize objectIndex, Args&&... args);

        void DestroyObjectBuffer();

        void DestroyGpuBuffer();
    };

    template <typename TObject>
    XObjectPoolBase<TObject>::XObjectPoolBase(
        cContext* context,
        types::boolean bKeepStagingBuffer,
        types::s32 gpuBufferSlot,
        cBuffer::eType gpuBufferType
    ) :
        iObject(context),
        _bKeepStagingBuffer(bKeepStagingBuffer),
        _gpuBufferSlot(gpuBufferSlot),
        _gpuBufferType(gpuBufferType) {}
    
    template <typename TObject>
    XObjectPoolBase<TObject>::~XObjectPoolBase()
    {
        if (_gpuBuffer)
            DestroyGpuBuffer();
        if (_staging)
            CObjectAllocator::Deallocate(_staging);
        if (_objects)
            DestroyObjectBuffer();
        if (_slots)
            CObjectAllocator::Deallocate(_slots);
    }

    template <typename TObject>
    void XObjectPoolBase<TObject>::Allocate(types::usize maxCount)
    {
        _lastObjectCursor = 0;
        _bDirtyBit = types::K_TRUE;

        const types::usize prevMaxCount = _reservedCount;
        _reservedCount = maxCount;

        if (_slots)
            ReallocatePodBuffer<SSlot>(_slots, prevMaxCount, maxCount);
        else
            AllocatePodBuffer(_slots, maxCount * sizeof(SSlot));

        if (_objects)
            ReallocateObjectBuffer(prevMaxCount, maxCount);
        else
            AllocatePodBuffer(_objects, maxCount * sizeof(TObject));
        
        if (_bKeepStagingBuffer == types::K_TRUE)
        {
            if (_staging)
                ReallocatePodBuffer<TObject::TGPULayout>(_staging, prevMaxCount, maxCount);
            else
                AllocatePodBuffer(_staging, maxCount * sizeof(TObject::TGPULayout));
        }
        else
        {
            _staging = nullptr;
        }

        auto AllocateGpuBuffer = [](
            cBuffer*& buffer,
            cContext* context,
            cBuffer::eType type,
            types::usize byteSize,
            types::s32 slot
        ) {
            context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
                ERenderCommand::CREATE_BUFFER,
                (types::cpuword)type,
                (types::cpuword)nullptr,
                byteSize,
                slot
            ));
            buffer = context->GetSubsystem<cEngine>()->
                GetSynchronization()->
                WaitForRenderCommandResult<cBuffer*>();
        };

        if (_gpuBuffer && _gpuBufferSlot > -1 && _gpuBufferType != cBuffer::eType::NONE)
        {
            DestroyGpuBuffer();
            AllocateGpuBuffer(
                _gpuBuffer,
                _context,
                _gpuBufferType,
                maxCount * sizeof(TObject::TGPULayout),
                _gpuBufferSlot
            );
            Upload();
        }
        else if (!_gpuBuffer && _gpuBufferSlot > -1 && _gpuBufferType != cBuffer::eType::NONE)
        {
            AllocateGpuBuffer(
                _gpuBuffer,
                _context,
                _gpuBufferType,
                maxCount * sizeof(TObject::TGPULayout),
                _gpuBufferSlot
            );
        }
    }

    template <typename TObject>
    template <typename... Args>
    std::optional<typename TObject::THandle> XObjectPoolBase<TObject>::Create(Args&&... args)
    {
        types::usize lastObjectIndex = 0;
        if (_freeSlots.empty())
        {
            lastObjectIndex = _lastObjectCursor++;
        }
        else
        {
            lastObjectIndex = _freeSlots.front();
            _freeSlots.pop();
        }

        OccupySlot(lastObjectIndex, _slots[lastObjectIndex].generation + 1);

        new (&_objects[lastObjectIndex]) TObject(std::forward<Args>(args)...);

        typename TObject::THandle handle;
        handle.index = lastObjectIndex;
        handle.generation = _slots[lastObjectIndex].generation;

        return handle;
    }

    template <typename TObject>
    std::optional<SObjectFrame<typename TObject::THandle>> XObjectPoolBase<TObject>::Create(types::usize count)
    {
        for (types::usize i = 0; i < _reservedCount; i++)
        {
            if (i + count > _reservedCount)
                return std::nullopt;

            types::boolean bFree = types::True;
            for (types::usize j = 0; j < count; j++)
            {
                if (_slots[i + j].alive == types::True)
                {
                    bFree = types::False;
                    break;
                }
            }

            if (bFree == types::True)
            {
                while (!_freeSlots.empty())
                    _freeSlots.pop();

                for (types::usize j = 0; j < count; j++)
                {
                    OccupySlot(i + j, _slots[i + j].generation + 1);

                    new (&_objects[i + j]) TObject();
                }

                for (types::usize j = 0; j < _lastObjectCursor; j++)
                    if (_slots[j].alive == types::False)
                        _freeSlots.push(j);

                if (i + count > _lastObjectCursor)
                    _lastObjectCursor = i + count;

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
    void XObjectPoolBase<TObject>::Destroy(const TObject::THandle& handle)
    {
        if (handle.generation != _slots[handle.index].generation ||
            _slots[handle.index].alive == types::False)
            return;

        _objects[handle.index].~TObject();
        _slots[handle.index].alive = types::False;
        _freeSlots.push(handle.index);
    }

    template <typename TObject>
    void XObjectPoolBase<TObject>::Destroy(const SObjectFrame<typename TObject::THandle>& frame)
    {
        for (types::usize i = 0; i < frame.count; i++)
        {
            _objects[frame.begin.index + i].~TObject();
            _slots[frame.begin.index + i].alive = types::False;
            _freeSlots.push(frame.begin.index + i);
        }
    }

    template <typename TObject>
    std::optional<std::reference_wrapper<TObject>> XObjectPoolBase<TObject>::Get(const TObject::THandle& handle)
    {
        if (handle.generation != _slots[handle.index].generation ||
            _slots[handle.index].alive == types::False)
            return std::nullopt;

        return _objects[handle.index];
    }

    template <typename TObject>
    void XObjectPoolBase<TObject>::WriteToStaging(types::usize bufferIndex, const TObject& object)
    {
        _staging[bufferIndex] = ConvertToGpuLayout(object);
        _bDirtyBit = types::K_TRUE;
    }

    template <typename TObject>
    types::usize XObjectPoolBase<TObject>::GetPackedIndex(const TObject::THandle& handle)
    {
        if (handle.generation != _slots[handle.index].generation ||
            _slots[handle.index].alive == types::False)
            return 0;

        types::usize packedIndex = 0;
        for (types::usize i = 0; i < handle.index; i++)
            if (_slots[i].alive == types::True)
                ++packedIndex;

        return packedIndex;
    }

    template <typename TObject>
    std::optional<typename TObject::THandle> XObjectPoolBase<TObject>::GetHandle(
        const TObject::THandle& baseHandle,
        const types::usize offset
    )
    {
        if (baseHandle.generation != _slots[baseHandle.index].generation ||
            _slots[baseHandle.index].alive == types::False)
            return std::nullopt;

        typename TObject::THandle offsetHandle;
        offsetHandle.index = baseHandle.index + offset;
        offsetHandle.generation = _slots[offsetHandle.index].generation;

        return offsetHandle;
    }

    template <typename TObject>
    void XObjectPoolBase<TObject>::Upload()
    {
        if (!_gpuBuffer || _gpuBufferSlot == -1 || _gpuBufferType == cBuffer::eType::NONE)
            return;

        if (_bDirtyBit == types::K_TRUE)
        {
            types::usize packedSize = 0;
            for (types::usize i = 0; i < _lastObjectCursor; i++)
                if (_slots[i].alive == types::True)
                    ++packedSize;

            _context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
                ERenderCommand::WRITE_BUFFER,
                (types::cpuword)_gpuBuffer,
                0,
                packedSize * sizeof(TObject::TGPULayout),
                (types::cpuword)&_staging[0]
            ));
            _context->GetSubsystem<cEngine>()->GetSynchronization()->WaitForRenderCommandResult<void*>();

            _bDirtyBit = types::K_FALSE;
        }
    }

    template <typename TObject>
    template <typename TBufferObject>
    void XObjectPoolBase<TObject>::AllocatePodBuffer(TBufferObject*& buffer, types::usize byteSize)
    {
        buffer = (TBufferObject*)CObjectAllocator::Allocate(byteSize, 64);
        memset(buffer, 0, byteSize);
    }

    template <typename TObject>
    template <typename TBufferObject>
    void XObjectPoolBase<TObject>::ReallocatePodBuffer(
        TBufferObject*& buffer,
        types::usize prevMaxCount,
        types::usize curMaxCount
    )
    {
        const types::usize objectByteSize = sizeof(TBufferObject);
        TBufferObject* temp = (TBufferObject*)CObjectAllocator::Allocate(curMaxCount * objectByteSize, 64);
        const types::usize copyElementSize = std::min(prevMaxCount, curMaxCount);
        memcpy(&temp[0], &buffer[0], copyElementSize * objectByteSize);
        CObjectAllocator::Deallocate(buffer);
        buffer = temp;
    }

    template <typename TObject>
    void XObjectPoolBase<TObject>::ReallocateObjectBuffer(types::usize prevMaxCount, types::usize curMaxCount)
    {
        const types::usize objectByteSize = sizeof(TObject);
        TObject* temp = (TObject*)CObjectAllocator::Allocate(curMaxCount * objectByteSize, 64);
        const types::usize copyElementSize = std::min(prevMaxCount, curMaxCount);
        for (types::usize slotIndex = 0; slotIndex < copyElementSize; slotIndex++)
        {
            if (_slots[slotIndex].alive == types::K_FALSE)
                continue;
            const types::usize objectIndex = slotIndex;
            if (objectIndex < copyElementSize)
                CObjectAllocator::Create(&temp[0], objectIndex, std::move(_objects[objectIndex]));
            _objects[objectIndex].~TObject();
        }
        CObjectAllocator::Deallocate(_objects);
        _objects = temp;
    }

    template <typename TObject>
    void XObjectPoolBase<TObject>::OccupySlot(types::usize objectIndex, types::usize generation)
    {
        _slots[objectIndex].generation = generation;
        _slots[objectIndex].alive = types::K_TRUE;
    }

    template <typename TObject>
    template <typename... Args>
    void XObjectPoolBase<TObject>::ConstructObject(types::usize objectIndex, Args&&... args)
    {
        CObjectAllocator::Create<TObject>(&_objects[0], objectIndex, std::forward<Args>(args)...);
    }

    template <typename TObject>
    void XObjectPoolBase<TObject>::DestroyObjectBuffer()
    {
        for (types::usize i = 0; i < _lastObjectCursor; i++)
            if (_slots[i].alive == types::K_TRUE)
                _objects[i].~TObject();
        CObjectAllocator::Deallocate(_objects);

        _lastObjectCursor = 0;
    }

    template <typename TObject>
    void XObjectPoolBase<TObject>::DestroyGpuBuffer()
    {
        if (!_gpuBuffer || _gpuBufferSlot < 0 || _gpuBufferType == cBuffer::eType::NONE)
            return;

        _context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
            ERenderCommand::DESTROY_BUFFER,
            (types::cpuword)_gpuBuffer
        ));
        _context->GetSubsystem<cEngine>()->GetSynchronization()->WaitForRenderCommandResult<void*>();
    }
}