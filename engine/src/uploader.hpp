// uploader.hpp

#pragma once

#include "object.hpp"
#include "context.hpp"
#include "engine.hpp"
#include "capabilities.hpp"
#include "graphics_resource_backend.hpp"
#include "gpu_resource.hpp"
#include "render_subsystem.hpp"
#include "subsystem.hpp"
#include "handle_allocator.hpp"
#include "types.hpp"

namespace triton
{
    template <typename TCPUObject, typename TCPUObjectHandle, typename TCPUObjectAllocator, typename TGPUElementLayout>
    class XUploader : public XHandleAllocator<SSlot, TCPUObject, TCPUObjectHandle, TCPUObjectAllocator>
    {
        TRITON_OBJECT(XUploader)

        types::boolean _bNeedsPersistentGpuWrite = types::K_FALSE;
        types::boolean _bNeedsGpuWrite = types::K_FALSE;
        cGPUResource* _resource = nullptr;
        types::usize _stagingBufferByteSize = 0;
        TGPUElementLayout* _stagingBuffer = nullptr;

    public:
        XUploader() = delete;

        explicit XUploader(
            cContext* context,
            cGPUResource* resource,
            types::usize stagingBufferElementCount,
            types::boolean bNeedsPersistentGpuWrite
        ) : XHandleAllocator(context)
        {
            const sCapabilities* caps = _context->GetSubsystem<cEngine>()->GetCapabilities();
            _bNeedsPersistentGpuWrite = bNeedsPersistentGpuWrite;
            _bNeedsGpuWrite = types::K_FALSE;
            _resource = resource;
            _stagingBufferByteSize = stagingBufferElementCount * sizeof(TGPUElementLayout);
            _stagingBuffer = (TGPUElementLayout*)_context->GetMemoryAllocator()->Allocate(_stagingBufferByteSize, 64);
        }

        ~XUploader() override
        {
            _context->GetMemoryAllocator()->Deallocate(_stagingBuffer);
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

        void UploadStagingToGpuIfDirty()
        {
            if (_bNeedsPersistentGpuWrite == types::K_TRUE ||
                _bNeedsGpuWrite == types::K_TRUE)
                UploadStagingToGpu(0, _stagingBufferByteSize);
        }

        void UploadStagingToGpu(types::usize byteOffset, types::usize byteSize)
        {
            if (_bNeedsGpuWrite == types::K_TRUE)
            {
                XRenderSubsystem* renderSubsystem = _context->GetSubsystem<XRenderSubsystem>();
                renderSubsystem->PushCommand(SRenderCommand(
                    ERenderCommand::WRITE_BUFFER,
                    (types::cpuword)_resource,
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