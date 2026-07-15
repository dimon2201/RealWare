// batch_storage.hpp

#pragma once

#include <optional>
#include "object.hpp"
#include "render_instance.hpp"
#include "buffer_view.hpp"
#include "handle_allocator.hpp"
#include "batch_instance.hpp"
#include "render_batch.hpp"
#include "handles.hpp"

namespace triton
{
    class cContext;
    struct SRenderBatchSlot;
    template <typename TValue>
    class XLinearArray;
    class SGeometryView;
    class XRenderBatch;

    class XBatchStorage : public iObject
    {
        TRITON_OBJECT(XBatchStorage)

        XHandleAllocator<SRenderBatchSlot, HBatch, XLinearArray<XRenderBatch>, XRenderBatch>* _batches = nullptr;

    public:
        explicit XBatchStorage(cContext* context);
        ~XBatchStorage() override;

        std::optional<HBatch> Create(const SGeometryView& geometry);
        XRenderBatch* Get(const HBatch& batch);
        void Remove(const HBatch& batch);
        std::optional<SBatchInstance> AddInstance(const HBatch& batch, ERenderInstanceMotionType usage);
        void RemoveInstance(const SBatchInstance& instance);

        inline SBufferView<XRenderBatch> GetBatches() const
        {
            return _batches->GetData();
        }
    };
}