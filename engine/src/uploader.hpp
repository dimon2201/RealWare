// uploader.hpp

#pragma once

#include "object.hpp"
#include "context.hpp"
#include "engine.hpp"
#include "capabilities.hpp"
#include "graphics_resource_backend.hpp"
#include "gpu_resource.hpp"
#include "subsystem.hpp"
#include "handle_allocator.hpp"
#include "object_allocator.hpp"
#include "synchronization.hpp"
#include "types.hpp"

namespace triton
{
    template <typename TCPUObject, typename TCPUObjectHandle, typename TCPUObjectAllocator, typename TGPUElementLayout>
    class CUploader : public CHandleAllocator<SSlot, TCPUObject, TCPUObjectHandle, TCPUObjectAllocator>
    {
        types::boolean _bNeedsPersistentGpuWrite = types::K_FALSE;
        types::boolean _bNeedsGpuWrite = types::K_FALSE;
        cGPUResource** _resource = nullptr;
        types::usize _stagingBufferByteSize = 0;
        TGPUElementLayout* _stagingBuffer = nullptr;

    public:
        CUploader() = delete;

        explicit CUploader(
            cContext* context,
            cGPUResource** resource,
            types::usize stagingBufferElementCount,
            types::boolean bNeedsPersistentGpuWrite
        ) : CHandleAllocator<SSlot, TCPUObject, TCPUObjectHandle, TCPUObjectAllocator>(context, 4096, 4096, 65536 * 64, 1024) // TODO: use values from capabilities here <<<<<-----
        {
            _bNeedsPersistentGpuWrite = bNeedsPersistentGpuWrite;
            _bNeedsGpuWrite = types::K_FALSE;
            _resource = resource;
            _stagingBufferByteSize = stagingBufferElementCount * sizeof(TGPUElementLayout);
            _stagingBuffer = (TGPUElementLayout*)CObjectAllocator::Allocate(_stagingBufferByteSize, 64);
        }

        ~CUploader()
        {
            CObjectAllocator::Deallocate(_stagingBuffer);
        }

        template <typename TElementField>
        void WriteFieldToStaging(
            const TCPUObjectHandle& object,
            types::usize gpuLayoutByteOffset,
            const TElementField& field
        )
        {
            const types::usize elementIndex = GetHandleBufferIndex(object);
            TElementField* ef = (TElementField*)((types::u8*)&_stagingBuffer[elementIndex] + gpuLayoutByteOffset);
            *ef = field;
            MarkDirty();
        }

        void WriteFieldToStaging(
            types::usize elementIndex,
            const TGPUElementLayout& gpuElementData
        )
        {
            _stagingBuffer[elementIndex] = gpuElementData;
            MarkDirty();
        }

        void WriteToStaging(
            types::usize elementOffset,
            const TGPUElementLayout* srcData,
            types::usize srcDataElementSize
        )
        {
            memcpy(&_stagingBuffer[elementOffset], &srcData[0], srcDataElementSize * sizeof(TGPUElementLayout));
            MarkDirty();
        }

        void UploadStagingToGpuIfDirty(XRenderCommandRecorder* commandRecorder)
        {
            if (_bNeedsPersistentGpuWrite == types::K_TRUE ||
                _bNeedsGpuWrite == types::K_TRUE)
                UploadStagingToGpu(0, _stagingBufferByteSize, commandRecorder);
        }

        void UploadStagingToGpu(types::usize byteOffset, types::usize byteSize, XRenderCommandRecorder* commandRecorder)
        {
            if (_bNeedsGpuWrite == types::K_TRUE)
            {
                commandRecorder->PushCommand(SRenderCommand(
                    ERenderCommand::WRITE_BUFFER,
                    (types::cpuword)*_resource,
                    byteOffset,
                    byteSize,
                    (types::cpuword)&_stagingBuffer[0]
                ));
                _bNeedsGpuWrite = types::K_FALSE;
            }
        }

    private:
        inline void MarkDirty()
        {
            _bNeedsGpuWrite = types::K_TRUE;
        }
    };
}