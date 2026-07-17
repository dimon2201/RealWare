// batcher.hpp

#pragma once

#include <optional>
#include "object.hpp"
#include "render_instance.hpp"
#include "buffer_view.hpp"
#include "handle_allocator.hpp"
#include "batch_data.hpp"
#include "handles.hpp"
#include "material_subsystem.hpp"
#include "skeleton_subsystem.hpp"
#include "types.hpp"

#include "DELETE_THIS_FILE_ASAP.hpp"

namespace triton
{
    class cContext;
    template <typename TValue>
    class XLinearArray;
    class SGeometryView;
    struct SBatchData;

    class XBatchSubsystem : public ISubsys,
                            public CUploader<SStaticRenderInstanceData, HStaticRenderInstance, XLinearArray<SStaticRenderInstanceData>, SGPUStaticRenderInstanceLayout>,
                            public CUploader<SDynamicRenderInstanceData, HDynamicRenderInstance, XLinearArray<SDynamicRenderInstanceData>, SGPUDynamicRenderInstanceLayout>
    {
        TRITON_OBJECT(XBatchSubsystem)

        CHandleAllocator<SSlot, SBatchData, HBatch, XLinearArray<SBatchData>>* _batches = nullptr;
        cBuffer* _staticGPUBuffer = nullptr;
        cBuffer* _dynamicGPUBuffer = nullptr;
        types::u32* _tempStaticCounterBuffer = nullptr;
        types::u32* _tempDynamicCounterBuffer = nullptr;
        types::boolean _bStaticBufferNeedsPacking = types::K_FALSE;
        types::boolean _bDynamicBufferNeedsPacking = types::K_FALSE;

    public:
        explicit XBatchSubsystem(cContext* context);
        ~XBatchSubsystem() override;

        std::optional<HBatch> Create(
            ERenderInstanceMotionType motionType,
            const SGeometryView& geometry
        );

        SBatchData& Get(const HBatch& batch);

        void Destroy(const HBatch& batch);

        HStaticRenderInstance AddStaticInstance(
            const HBatch& batch,
            const HGameObject& gameObject
        );

        HDynamicRenderInstance AddDynamicInstance(
            const HBatch& batch,
            const HGameObject& gameObject
        );

        void RemoveStaticInstance(const HStaticRenderInstance& instance);

        void RemoveDynamicInstance(const HDynamicRenderInstance& instance);

        std::optional<HBatch> FindSimilarBatch(
            const types::u8* vertexBytes,
            types::usize vertexBytesCount,
            const types::u8* indexBytes,
            types::usize indexBytesCount
        );

        void Init() override {}
        void Free() override {}
        void Update() override;

        inline const SBufferView<SBatchData>& GetBatches() const
        {
            return _batches->GetData();
        }

        inline cBuffer& GetStaticInstanceGPUBuffer() const
        {
            return *_staticGPUBuffer;
        }

        inline cBuffer& GetDynamicInstanceGPUBuffer() const
        {
            return *_dynamicGPUBuffer;
        }

    private:
        void MarkDirtyStatic();

        void MarkDirtyDynamic();

        types::boolean StaticBufferNeedsPacking();

        types::boolean DynamicBufferNeedsPacking();

        void RecalcBufferOffsetsAndResetCounters(types::u32* counterBuffer);

        void PackInstancesToStagingBuffers(
            ERenderInstanceMotionType motionType,
            types::u32* counterBuffer
        );
    };
}