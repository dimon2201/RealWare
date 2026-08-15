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

    template <typename TObject>
    struct SObjectFrame
    {
        typename TObject::THandle begin;
        typename TObject::THandle end;
    };

    template <typename TObject>
    class XObjectPoolBase : public iObject
    {
        TRITON_OBJECT(XObjectPoolBase)

    protected:
        types::boolean _bKeepStagingBuffer = types::K_FALSE;
        SSlot* _slots = nullptr;
        TObject* _objects = nullptr;
        std::queue<types::usize> _freeSlots = {};
        types::usize _allocatedObjectCount = 0;
        types::usize _objectCount = 0;
        cBuffer* _gpuBuffer = nullptr;
        types::s32 _gpuBufferSlot = -1;
        cBuffer::eType _gpuBufferType = cBuffer::eType::NONE;
        TObject::TGPULayout* _stagingBuffer = nullptr;
        types::boolean _bDirtyBit = types::K_FALSE;

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

        template <typename... Args>
        std::optional<SObjectFrame<TObject>> Create(types::usize count, Args&&... args);

        void Destroy(const TObject::THandle& handle);

        void Destroy(const SObjectFrame<TObject>& frame);

        std::optional<std::reference_wrapper<TObject>> Get(const TObject::THandle& handle);

        std::optional<SBufferView<TObject>> Get(const SObjectFrame<TObject>& frame);

        void WriteToStaging(types::usize bufferIndex, const TObject& object);

        types::usize GetBufferIndex(const TObject::THandle& handle);

        types::usize GetPackedBufferIndex(const TObject::THandle& handle);

        std::optional<typename TObject::THandle> GetHandle(const types::usize& bufferIndex);

        void Upload();

        inline types::usize GetSize() const
        {
            return _objectCount;
        }

        inline SBufferView<TObject> GetData() const
        {
            return SBufferView<TObject>(&_objects[0], sizeof(TObject) * _objectCount);
        }

        inline cBuffer* GetGPUBuffer() const
        {
            return _gpuBuffer;
        }

        virtual TObject::TGPULayout ConvertToGpuLayout(const TObject& object) = 0;

        virtual void Update() = 0;

    private:
        types::boolean CheckBufferOverflow(types::usize count);

        template <typename TBufferObject>
        void AllocatePodBuffer(TBufferObject*& buffer, types::usize byteSize);

        template <typename TBufferObject>
        void ReallocatePodBuffer(
            TBufferObject*& buffer,
            types::usize prevMaxCount,
            types::usize curMaxCount
        );

        void ReallocateObjectBuffer(types::usize prevMaxCount, types::usize curMaxCount);

        void SetSlot(types::usize objectIndex, types::usize generation);
        
        template <typename... Args>
        void CreateObjectAt(types::usize objectIndex, Args&&... args);

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
        if (_stagingBuffer)
            CObjectAllocator::Deallocate(_stagingBuffer);
        if (_objects)
            DestroyObjectBuffer();
        if (_slots)
            CObjectAllocator::Deallocate(_slots);
    }

    template <typename TObject>
    void XObjectPoolBase<TObject>::Allocate(types::usize maxCount)
    {
        _objectCount = 0;
        _bDirtyBit = types::K_TRUE;

        const types::usize prevMaxCount = _allocatedObjectCount;
        _allocatedObjectCount = maxCount;

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
            if (_stagingBuffer)
                ReallocatePodBuffer<TObject::TGPULayout>(_stagingBuffer, prevMaxCount, maxCount);
            else
                AllocatePodBuffer(_stagingBuffer, maxCount * sizeof(TObject::TGPULayout));
        }
        else
        {
            _stagingBuffer = nullptr;
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
        if (CheckBufferOverflow(1) == types::K_TRUE)
            return std::nullopt;

        const types::usize lastObjectIndex = _objectCount++;

        types::usize slotIndex;
        types::usize arrayIndex;
        types::usize generation;
        if (_freeSlots.empty())
        {
            slotIndex = lastObjectIndex;
            generation = 0;

            SetSlot(lastObjectIndex, generation);
        }
        else
        {
            const types::usize freeIndex = _freeSlots.front();
            _freeSlots.pop();

            slotIndex = freeIndex;
            generation = _slots[freeIndex]._generation + 1;
            
            SetSlot(freeIndex, generation);
        }

        arrayIndex = slotIndex;

        CreateObjectAt(arrayIndex, std::forward<Args>(args)...);

        typename TObject::THandle handle;
        handle._slotIndex = slotIndex;
        handle._indexInArray = arrayIndex;
        handle._generation = generation;

        return handle;
    }

    template <typename TObject>
    template <typename... Args>
    std::optional<SObjectFrame<TObject>> XObjectPoolBase<TObject>::Create(
        types::usize count,
        Args&&... args
    )
    {
        for (types::usize i = 0; i < _allocatedObjectCount; i++)
        {
            if (i + count <= _allocatedObjectCount)
            {
                types::boolean bFree = types::K_TRUE;
                for (types::usize j = 0; j < count; j++)
                {
                    if (_slots[i + j]._alive == types::K_TRUE)
                    {
                        bFree = types::K_FALSE;
                        break;
                    }
                }

                if (bFree == types::K_TRUE)
                {
                    typename TObject::THandle begin;
                    begin._slotIndex = i;
                    begin._indexInArray = i;
                    begin._generation = _slots[i]._generation;

                    typename TObject::THandle end;
                    end._slotIndex = i + (count - 1);
                    end._indexInArray = i + (count - 1);
                    end._generation = _slots[i + (count - 1)]._generation;

                    SObjectFrame<TObject> frame;
                    frame.begin = begin;
                    frame.end = end;

                    return frame;
                }
            }
        }

        return std::nullopt;
    }

    template <typename TObject>
    void XObjectPoolBase<TObject>::Destroy(const TObject::THandle& handle)
    {
        if (handle.IsInvalid() ||
            handle._generation != _slots[handle._slotIndex]._generation ||
            _slots[handle._slotIndex]._alive == types::K_FALSE)
            return;

        const types::usize objectIndex = GetBufferIndex(handle);
        _slots[objectIndex]._alive = types::K_FALSE;
        _freeSlots.push(objectIndex);
    }

    template <typename TObject>
    void XObjectPoolBase<TObject>::Destroy(const SObjectFrame<TObject>& frame)
    {
        if (frame.begin.IsInvalid() ||
            frame.end.IsInvalid() ||
            frame.begin._generation != _slots[frame.begin._slotIndex]._generation ||
            frame.end._generation != _slots[frame.end._slotIndex]._generation ||
            _slots[frame.begin._slotIndex]._alive == types::K_FALSE ||
            _slots[frame.end._slotIndex]._alive == types::K_FALSE)
            return;

        const types::usize beginObjectIndex = GetBufferIndex(frame.begin);
        const types::usize endObjectIndex = GetBufferIndex(frame.end);
        for (types::usize i = beginObjectIndex; i <= endObjectIndex; i++)
        {
            _slots[i]._alive = types::K_FALSE;
            _freeSlots.push(i);
        }
    }

    template <typename TObject>
    std::optional<std::reference_wrapper<TObject>> XObjectPoolBase<TObject>::Get(const TObject::THandle& handle)
    {
        if (handle.IsInvalid() ||
            _slots[handle._slotIndex]._alive == types::K_FALSE ||
            handle._generation != _slots[handle._slotIndex]._generation)
        {
            return std::nullopt;
        }
        else
        {
            return _objects[GetBufferIndex(handle)];
        }
    }

    template <typename TObject>
    std::optional<SBufferView<TObject>> XObjectPoolBase<TObject>::Get(const SObjectFrame<TObject>& frame)
    {
        if (frame.begin.IsInvalid() ||
            frame.end.IsInvalid() ||
            frame.begin._generation != _slots[frame.begin._slotIndex]._generation ||
            frame.end._generation != _slots[frame.end._slotIndex]._generation ||
            _slots[frame.begin._slotIndex]._alive == types::K_FALSE ||
            _slots[frame.end._slotIndex]._alive == types::K_FALSE)
            return std::nullopt;

        const types::usize beginObjectIndex = GetBufferIndex(frame.begin);
        const types::usize endObjectIndex = GetBufferIndex(frame.end);
        const types::usize count = (endObjectIndex - beginObjectIndex) + 1;

        SBufferView<TObject> bv;
        bv.elements = &_objects[beginObjectIndex];
        bv.byteSize = count * sizeof(TObject);
        bv.elementCount = count;

        return bv;
    }

    template <typename TObject>
    void XObjectPoolBase<TObject>::WriteToStaging(types::usize bufferIndex, const TObject& object)
    {
        if (bufferIndex >= _allocatedObjectCount)
            return;

        _stagingBuffer[bufferIndex] = ConvertToGpuLayout(object);

        _bDirtyBit = types::K_TRUE;
    }

    template <typename TObject>
    types::usize XObjectPoolBase<TObject>::GetBufferIndex(const TObject::THandle& handle)
    {
        if (handle.IsInvalid() ||
            _slots[handle._slotIndex]._alive == types::K_FALSE)
            return 0;

        return _slots[handle._slotIndex]._arrayIndex;
    }

    template <typename TObject>
    types::usize XObjectPoolBase<TObject>::GetPackedBufferIndex(const TObject::THandle& handle)
    {
        if (handle.IsInvalid() ||
            _slots[handle._slotIndex]._alive == types::K_FALSE)
            return 0;

        const types::usize realIndex = _slots[handle._slotIndex]._arrayIndex;
        types::usize packedIndex = 0;
        for (types::usize i = 0; i < realIndex; i++)
        {
            if (_slots[i]._alive == types::K_TRUE)
                packedIndex++;
        }

        return packedIndex;
    }

    template <typename TObject>
    std::optional<typename TObject::THandle> XObjectPoolBase<TObject>::GetHandle(const types::usize& bufferIndex)
    {
        if (bufferIndex >= _allocatedObjectCount)
            return std::nullopt;

        typename TObject::THandle handle;
        handle.Invalidate();
        handle._slotIndex = bufferIndex;
        handle._indexInArray = bufferIndex;
        handle._generation = _slots[handle._slotIndex]._generation;

        return handle;
    }

    template <typename TObject>
    void XObjectPoolBase<TObject>::Upload()
    {
        if (!_gpuBuffer || _gpuBufferSlot == -1 || _gpuBufferType == cBuffer::eType::NONE)
            return;

        if (_bDirtyBit == types::K_TRUE)
        {
            _context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
                ERenderCommand::WRITE_BUFFER,
                (types::cpuword)_gpuBuffer,
                0,
                _objectCount * sizeof(TObject::TGPULayout),
                (types::cpuword)&_stagingBuffer[0]
            ));
            _context->GetSubsystem<cEngine>()->GetSynchronization()->WaitForRenderCommandResult<void*>();

            _bDirtyBit = types::K_FALSE;
        }
    }

    template <typename TObject>
    types::boolean XObjectPoolBase<TObject>::CheckBufferOverflow(types::usize count)
    {
        if (_objectCount + count > _allocatedObjectCount)
        {
            Print("Error: object pool buffer overflow, count = " + std::to_string(_objectCount + count));
            return types::K_TRUE;
        }
        else
        {
            return types::K_FALSE;
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
            if (_slots[slotIndex]._alive == types::K_FALSE)
                continue;
            const types::usize objectIndex = _slots[slotIndex]._arrayIndex;
            if (objectIndex < copyElementSize)
                CObjectAllocator::Create(&temp[0], objectIndex, std::move(_objects[objectIndex]));
            _objects[objectIndex].~TObject();
        }
        CObjectAllocator::Deallocate(_objects);
        _objects = temp;
    }

    template <typename TObject>
    void XObjectPoolBase<TObject>::SetSlot(types::usize objectIndex, types::usize generation)
    {
        _slots[objectIndex]._arrayIndex = objectIndex;
        _slots[objectIndex]._generation = generation;
        _slots[objectIndex]._alive = types::K_TRUE;
    }

    template <typename TObject>
    template <typename... Args>
    void XObjectPoolBase<TObject>::CreateObjectAt(types::usize objectIndex, Args&&... args)
    {
        CObjectAllocator::Create(&_objects[0], objectIndex, std::forward<Args>(args)...);
    }

    template <typename TObject>
    void XObjectPoolBase<TObject>::DestroyObjectBuffer()
    {
        for (types::usize i = 0; i < _objectCount; i++)
            if (_slots[i]._alive == types::K_TRUE)
                _objects[i].~TObject();
        CObjectAllocator::Deallocate(_objects);

        _objectCount = 0;
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