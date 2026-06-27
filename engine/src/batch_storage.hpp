// batch_storage.hpp

#pragma once

#include <optional>
#include "object.hpp"

namespace triton
{
    class cContext;
    struct SRenderBatchSlot;
    struct SRenderBatchHandle;
    template <typename TValue>
    class cStack;
    class XRenderBatch;
    class SGeometryView;
    template <typename TSlot, typename THandle, typename TDataStructure, typename TObject>
    class XHandleAllocator;

    class XBatchStorage : public iObject
    {
        TRITON_OBJECT(XBatchStorage)

        XHandleAllocator<SRenderBatchSlot, SRenderBatchHandle, cStack<XRenderBatch>, XRenderBatch>* _batches = nullptr;

    public:
        explicit XBatchStorage(cContext* context);
        ~XBatchStorage() override;

        std::optional<SRenderBatchHandle> Create(const SGeometryView& geometry);
        void Remove(const SRenderBatchHandle& batch);
    };
}