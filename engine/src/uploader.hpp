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
#include "types.hpp"

namespace triton
{
    template <typename TCPUStorage, typename TCPUObjectHandle, typename TGPUElementLayout>
    class XUploader : public iObject
    {
        TRITON_OBJECT(XUploader)

        types::boolean _bIsPermanentDirty = types::K_FALSE;
        types::boolean _bIsDirty = types::K_FALSE;
        cGPUResource* _resource = nullptr;
        types::usize _stagingBufferByteSize = 0;
        TGPUElementLayout* _stagingBuffer = nullptr;

    public:
        explicit XUploader(
            cContext* context,
            cGPUResource* resource,
            types::usize stagingBufferElementCount,
            types::boolean bIsPermanentDirty
        ) : iObject(context)
        {
            const sCapabilities* caps = _context->GetSubsystem<cEngine>()->GetCapabilities();
            _bIsPermanentDirty = bIsPermanentDirty;
            _resource = resource;
            _stagingBufferByteSize = stagingBufferElementCount * sizeof(TGPUElementLayout);
            _stagingBuffer = (TGPUElementLayout*)_context->GetMemoryAllocator()->Allocate(_stagingBufferByteSize, 64);
        }

        ~XUploader() override
        {
            _context->GetMemoryAllocator()->Deallocate(_stagingBuffer);
        }

        template <typename TElementField>
        void WriteField(
            const TCPUObjectHandle& object,
            types::usize gpuLayoutByteOffset,
            const TElementField& field
        )
        {
            const types::usize elementIndex = _context->GetSubsystem<TSubsystem>()->GetBufferIndex(object);
            TElementField* ef = (TElementField*)((types::u8*)&_stagingBuffer[elementIndex] + gpuLayoutByteOffset);
            *ef = field;
            MarkDirty();
        }

        void Update()
        {
            if (_bIsPermanentDirty == types::K_TRUE || _bIsDirty == types::K_TRUE)
                Upload(0, _stagingBufferByteSize);
        }

        void Upload(types::usize byteOffset, types::usize byteSize)
        {
            if (_bIsDirty == K_TRUE)
            {
                XRenderSubsystem* renderSubsystem = _context->GetSubsystem<XRenderSubsystem>();
                renderSubsystem->PushCommand(SRenderCommand(
                    ERenderCommand::WRITE_BUFFER,
                    (cpuword)_resource,
                    byteOffset,
                    byteSize,
                    (cpuword)&_stagingBuffer[0]
                ));
                _bIsDirty = types::K_FALSE;
            }
        }

    private:
        void MarkDirty()
        {
            _bIsDirty = types::K_TRUE;
        }
    };
}