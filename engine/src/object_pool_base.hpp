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
#include "material.hpp"
#include "synchronization.hpp"
#include "types.hpp"

namespace triton
{
    class cContext;

    template <typename TObject>
    struct SObjectFrame
    {
        std::string label = "";
        std::vector<typename TObject::THandle> handles;
    };

    struct SObjectFrameBounds
    {
        SObjectFrameBounds() = default;
        SObjectFrameBounds(
            const std::string& label,
            types::u32 begin,
            types::u32 end
        ) : label(label), begin(begin), end(end) {}

        std::string label = "";
        types::u32 begin = 0;
        types::u32 end = 0;
    };

    template <typename TObject>
    class XObjectPoolBase : public iObject
    {
        TRITON_OBJECT(XObjectPoolBase)

    protected:
        types::boolean _bKeepCpuCopy = types::K_FALSE;
        SSlot* _slots = nullptr;
        types::usize* _objectIndexToSlotIndex = nullptr;
        std::queue<types::usize> _freeSlots;
        TObject* _objects = nullptr;
        std::vector<SObjectFrameBounds> _frames = {};
        types::usize _allocatedObjectCounter = 0;
        types::usize _slotCounter = 0;
        types::usize _objectCounter = 0;
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

        // NOTE: Can be fragmented, use carefully
        template <typename... Args>
        std::optional<SObjectFrame<TObject>> Create(const std::string& label, types::usize count, Args&&... args);

        void Destroy(const TObject::THandle& handle);

        void Destroy(const SObjectFrame<TObject>& frame);

        std::optional<std::reference_wrapper<TObject>> Get(const TObject::THandle& handle);

        std::optional<SObjectFrame<TObject>> Find(const std::string& label);

        void WriteToStaging(const TObject::THandle& handle);

        void WriteToStaging(
            const SObjectFrame<TObject>& frame,
            const TObject::TGPULayout* data,
            types::usize dataByteSize
        );

        std::optional<types::usize> GetBufferIndex(const TObject::THandle& handle);

        std::optional<typename TObject::THandle> GetHandle(const types::usize& bufferIndex);

        void Upload();

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

        void CreateSlot(types::usize slotIndex, types::usize arrayIndex, types::usize generation);

        void SetSlot(types::usize slotIndex, types::usize arrayIndex, types::usize generation);
        
        template <typename... Args>
        void CreateObject(types::usize slotIndex, types::usize arrayIndex, Args&&... args);

        void DestroyObjectBuffer();

        void DestroyGpuBuffer();
    };

    template <typename TObject>
    XObjectPoolBase<TObject>::XObjectPoolBase(
        cContext* context,
        types::boolean bKeepCpuCopy,
        types::s32 gpuBufferSlot,
        cBuffer::eType gpuBufferType
    ) : iObject(context), _bKeepCpuCopy(bKeepCpuCopy), _gpuBufferSlot(gpuBufferSlot), _gpuBufferType(gpuBufferType) {}
    
    template <typename TObject>
    XObjectPoolBase<TObject>::~XObjectPoolBase()
    {
        if (_gpuBuffer)
            DestroyGpuBuffer();
        if (_stagingBuffer)
            CObjectAllocator::Deallocate(_stagingBuffer);
        if (_objects)
            DestroyObjectBuffer();
        if (_objectIndexToSlotIndex)
            CObjectAllocator::Deallocate(_objectIndexToSlotIndex);
        if (_slots)
            CObjectAllocator::Deallocate(_slots);
    }

    template <typename TObject>
    void XObjectPoolBase<TObject>::Allocate(types::usize maxCount)
    {
        while (!_freeSlots.empty())
            _freeSlots.pop();
        _slotCounter = 0;
        _objectCounter = 0;

        const types::usize prevMaxCount = _allocatedObjectCounter;
        _allocatedObjectCounter = maxCount;

        if (_slots)
            ReallocatePodBuffer<SSlot>(_slots, prevMaxCount, maxCount);
        else
            AllocatePodBuffer(_slots, maxCount * sizeof(SSlot));
        if (_objectIndexToSlotIndex)
            ReallocatePodBuffer<types::usize>(_objectIndexToSlotIndex, prevMaxCount, maxCount);
        else
            AllocatePodBuffer(_objectIndexToSlotIndex, maxCount * sizeof(types::usize));
        if (_bKeepCpuCopy == types::K_TRUE)
        {
            if (_objects)
                ReallocateObjectBuffer(prevMaxCount, maxCount);
            else
                AllocatePodBuffer(_objects, maxCount * sizeof(TObject));
        }
        else
        {
            _objects = nullptr;
        }
        if (_stagingBuffer)
            ReallocatePodBuffer<TObject::TGPULayout>(_stagingBuffer, prevMaxCount, maxCount);
        else
            AllocatePodBuffer(_stagingBuffer, maxCount * sizeof(TObject::TGPULayout));

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

        _bDirtyBit = types::K_TRUE;
    }

    template <typename TObject>
    template <typename... Args>
    std::optional<typename TObject::THandle> XObjectPoolBase<TObject>::Create(Args&&... args)
    {
        if (!_slots || (_bKeepCpuCopy == types::K_TRUE && !_objects))
            return std::nullopt;

        if (_freeSlots.empty() && CheckBufferOverflow(1) == types::K_TRUE)
            return std::nullopt;

        types::usize slotIndex;
        types::usize arrayIndex;
        types::usize generation;
        if (_freeSlots.empty())
        {
            slotIndex = _slotCounter;
            arrayIndex = _objectCounter;
            generation = 0;

            CreateSlot(slotIndex, arrayIndex, generation);
        }
        else
        {
            slotIndex = _freeSlots.front();
            _freeSlots.pop();
            arrayIndex = _objectCounter;
            generation = _slots[slotIndex]._generation;

            SetSlot(slotIndex, arrayIndex, generation);
        }

        if (_bKeepCpuCopy == types::K_TRUE)
            CreateObject(slotIndex, _slots[slotIndex]._arrayIndex, std::forward<Args>(args)...);

        typename TObject::THandle handle;
        handle._slotIndex = slotIndex;
        handle._generation = _slots[slotIndex]._generation;

        WriteToStaging(handle);

        return handle;
    }

    template <typename TObject>
    template <typename... Args>
    std::optional<SObjectFrame<TObject>> XObjectPoolBase<TObject>::Create(
        const std::string& label,
        types::usize count,
        Args&&... args
    )
    {
        if (!_slots || (_bKeepCpuCopy == types::K_TRUE && !_objects))
            return std::nullopt;

        if (CheckBufferOverflow(count) == types::K_TRUE)
            return std::nullopt;

        SObjectFrame<TObject> frame;
        frame.label = label;

        for (types::usize i = 0; i < count; i++)
        {
            auto opt = Create(std::forward<Args>(args)...);
            if (!opt)
                return std::nullopt;
            frame.handles.push_back(*opt);
        }

        if (frame.handles.empty())
            return std::nullopt;

        _frames.push_back({ GetBufferIndex(frame.handles.front()), GetBufferIndex(frame.handles.back()) });

        return frame;
    }

    template <typename TObject>
    void XObjectPoolBase<TObject>::Destroy(const TObject::THandle& handle)
    {
        if (handle.IsInvalid() ||
            handle._generation != _slots[handle._slotIndex]._generation ||
            _slots[handle._slotIndex]._alive == types::K_FALSE ||
            !_objectCounter)
            return;

        const types::usize curSlotIndex = handle._slotIndex;
        const types::usize curObjectIndex = _slots[curSlotIndex]._arrayIndex;
        const types::usize lastObjectIndex = _objectCounter - 1;

        if (_bKeepCpuCopy == types::K_TRUE)
        {
            _objects[curObjectIndex].~TObject();
            if (curObjectIndex != lastObjectIndex)
            {
                CObjectAllocator::Create(&_objects[0], curObjectIndex, std::move(_objects[lastObjectIndex]));
                _objects[lastObjectIndex].~TObject();

                const types::usize lastSlotIndex = _objectIndexToSlotIndex[lastObjectIndex];
                _objectIndexToSlotIndex[curObjectIndex] = lastSlotIndex;
                _slots[lastSlotIndex]._arrayIndex = curObjectIndex;
            }
            --_objectCounter;
        }

        _freeSlots.push(curSlotIndex);

        WriteToStaging(handle);

        ++_slots[curSlotIndex]._generation;
        _slots[curSlotIndex]._alive = types::K_FALSE;
    }

    template <typename TObject>
    void XObjectPoolBase<TObject>::Destroy(const SObjectFrame<TObject>& frame)
    {
        if (frame.handles.empty())
            return;

        SObjectFrameBounds ofb(
            frame.label,
            *GetBufferIndex(frame.handles.front()),
            *GetBufferIndex(frame.handles.back())
        );

        for (auto& handle : frame.handles)
            Destroy(handle);

        auto foundIt = _frames.end();
        for (auto it = _frames.begin(); it != _frames.end();)
        {
            if (it->begin == ofb.begin && it->end == ofb.end)
            {
                foundIt = _frames.erase(it);
                break;
            }
            else
            {
                ++it;
            }
        }
        types::u32 subCount = ofb.end - ofb.begin + 1;
        for (; foundIt != _frames.end(); ++foundIt)
        {
            foundIt->begin -= subCount;
            foundIt->end -= subCount;
        }
    }

    template <typename TObject>
    std::optional<std::reference_wrapper<TObject>> XObjectPoolBase<TObject>::Get(const TObject::THandle& handle)
    {
        if (_bKeepCpuCopy == types::K_FALSE)
            return std::nullopt;

        if (handle.IsInvalid() ||
            _slots[handle._slotIndex]._alive == types::K_FALSE ||
            handle._generation != _slots[handle._slotIndex]._generation)
        {
            return std::nullopt;
        }
        else
        {
            auto opt = GetBufferIndex(handle);
            if (opt.has_value())
                return _objects[*opt];
            else
                return std::nullopt;
        }
    }

    template <typename TObject>
    std::optional<SObjectFrame<TObject>> XObjectPoolBase<TObject>::Find(const std::string& label)
    {
        if (_frames.empty())
            return std::nullopt;

        types::boolean bFound = types::K_FALSE;
        auto foundFrame = _frames.end();
        for (auto it = _frames.begin(); it != _frames.end(); ++it)
        {
            if (it->label == label)
            {
                foundFrame = it;
                bFound = types::K_TRUE;
                break;
            }
        }

        if (bFound == types::K_FALSE)
            return std::nullopt;

        SObjectFrame<TObject> of;
        of.label = label;
        types::u32 begin = foundFrame->begin;
        types::u32 end = foundFrame->end;
        for (types::u32 i = begin; i < end; i++)
            of.handles.push_back(*GetHandle(i));

        return of;
    }

    template <typename TObject>
    void XObjectPoolBase<TObject>::WriteToStaging(const TObject::THandle& handle)
    {
        if (!_stagingBuffer)
            return;

        auto optIndex = GetBufferIndex(handle);
        auto optObject = Get(handle);
        if (optIndex.has_value() && optObject.has_value())
            _stagingBuffer[*optIndex] = ConvertToGpuLayout(*optObject);
        else
            return;

        _bDirtyBit = types::K_TRUE;
    }

    template <typename TObject>
    void XObjectPoolBase<TObject>::WriteToStaging(
        const SObjectFrame<TObject>& frame,
        const TObject::TGPULayout* data,
        types::usize dataByteSize
    )
    {
        if (!_stagingBuffer ||
            frame.handles.size() * sizeof(TObject::TGPULayout) != dataByteSize)
            return;

        types::usize counter = 0;
        for (auto& handle : frame.handles)
        {
            if (handle.IsInvalid())
                return;
            auto optIndex = GetBufferIndex(handle);
            if (optIndex.has_value())
                _stagingBuffer[*optIndex] = data[counter++];
            else
                return;
        }

        _bDirtyBit = types::K_TRUE;
    }

    template <typename TObject>
    std::optional<types::usize> XObjectPoolBase<TObject>::GetBufferIndex(const TObject::THandle& handle)
    {
        if (handle.IsInvalid() ||
            _slots[handle._slotIndex]._alive == types::K_FALSE)
            return std::nullopt;

        return _slots[handle._slotIndex]._arrayIndex;
    }

    template <typename TObject>
    std::optional<typename TObject::THandle> XObjectPoolBase<TObject>::GetHandle(const types::usize& bufferIndex)
    {
        if (bufferIndex >= _allocatedObjectCounter)
            return std::nullopt;

        TObject::THandle handle;
        handle.Invalidate();
        handle._slotIndex = _objectIndexToSlotIndex[bufferIndex];
        handle._generation = _slots[handle._slotIndex]._generation;
        handle._indexInArray = bufferIndex;

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
                _objectCounter * sizeof(TObject::TGPULayout),
                (types::cpuword)&_stagingBuffer[0]
            ));
            _context->GetSubsystem<cEngine>()->GetSynchronization()->WaitForRenderCommandResult<void*>();

            _bDirtyBit = types::K_FALSE;
        }
    }

    template <typename TObject>
    types::boolean XObjectPoolBase<TObject>::CheckBufferOverflow(types::usize count)
    {
        if (_objectCounter + count > _allocatedObjectCounter)
        {
            Print("Error: object pool buffer overflow, count = " + std::to_string(_objectCounter + count));
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
        if (_bKeepCpuCopy == types::K_FALSE)
            return;

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
    void XObjectPoolBase<TObject>::CreateSlot(types::usize slotIndex, types::usize arrayIndex, types::usize generation)
    {
        _slots[slotIndex]._arrayIndex = arrayIndex;
        _slots[slotIndex]._generation = generation;
        _slots[slotIndex]._alive = types::K_TRUE;
        ++_slotCounter;
    }

    template <typename TObject>
    void XObjectPoolBase<TObject>::SetSlot(types::usize slotIndex, types::usize arrayIndex, types::usize generation)
    {
        _slots[slotIndex]._arrayIndex = arrayIndex;
        _slots[slotIndex]._generation = generation;
        _slots[slotIndex]._alive = types::K_TRUE;
    }

    template <typename TObject>
    template <typename... Args>
    void XObjectPoolBase<TObject>::CreateObject(types::usize slotIndex, types::usize arrayIndex, Args&&... args)
    {
        if (_bKeepCpuCopy == types::K_FALSE)
            return;

        _objectIndexToSlotIndex[arrayIndex] = slotIndex;
        CObjectAllocator::Create(&_objects[0], arrayIndex, std::forward<Args>(args)...);

        ++_objectCounter;
    }

    template <typename TObject>
    void XObjectPoolBase<TObject>::DestroyObjectBuffer()
    {
        if (_bKeepCpuCopy == types::K_FALSE)
            return;

        for (types::usize i = 0; i < _slotCounter; i++)
            if (_slots[i]._alive == types::K_TRUE)
                _objects[_slots[i]._arrayIndex].~TObject();
        CObjectAllocator::Deallocate(_objects);

        _slotCounter = 0;
        _objectCounter = 0;
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