// batch_storage.hpp

#pragma once

#include <optional>
#include "object.hpp"
#include "render_instance.hpp"
#include "buffer_view.hpp"
#include "handle_allocator.hpp"

namespace triton
{
    class cContext;
    struct SRenderBatchSlot;
    struct SRenderBatchHandle;
    template <typename TValue>
    class XLinearArray;
    class XRenderBatch;
    class SGeometryView;
    class SInstanceBufferHandle;

    class XBatchStorage : public iObject
    {
        TRITON_OBJECT(XBatchStorage)

        XHandleAllocator<SRenderBatchSlot, SRenderBatchHandle, XLinearArray<XRenderBatch>, XRenderBatch>* _batches = nullptr;

    public:
        explicit XBatchStorage(cContext* context);
        ~XBatchStorage() override;

        std::optional<SRenderBatchHandle> Create(const SGeometryView& geometry);
        XRenderBatch* Get(const SRenderBatchHandle& batch);
        void Remove(const SRenderBatchHandle& batch);
        std::optional<SInstanceBufferHandle> AddInstance(const SRenderBatchHandle& batch, SRenderInstance::EUsage usage);
        void RemoveInstance(const SInstanceBufferHandle& instance);

        inline SBufferView<XRenderBatch> GetBatches() const
        {
            return _batches->GetData();
        }
    };
}