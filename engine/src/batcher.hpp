// batcher.hpp

#pragma once

#include <optional>
#include "object.hpp"
#include "render_instance.hpp"
#include "buffer_view.hpp"
#include "handle_allocator.hpp"
#include "batch_data.hpp"
#include "handles.hpp"
#include "static_instance_storage.hpp"
#include "dynamic_instance_storage.hpp"
#include "types.hpp"

namespace triton
{
    class cContext;
    template <typename TValue>
    class XLinearArray;
    class SGeometryView;
    struct SBatchData;

    class XBatchSubsystem : public iObject
    {
        TRITON_OBJECT(XBatchSubsystem)

        CHandleAllocator<SSlot, SBatchData, HBatch, XLinearArray<SBatchData>>* _batches = nullptr;
        cBuffer* _staticGPUBuffer = nullptr;
        cBuffer* _dynamicGPUBuffer = nullptr;
        CStaticInstanceStorage* _staticStorage = nullptr;
        CDynamicInstanceStorage* _dynamicStorage = nullptr;
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

        HRenderInstance AddStaticInstance(
            const HBatch& batch,
            const HGameObject& gameObject
        );

        HRenderInstance AddDynamicInstance(
            const HBatch& batch,
            const HGameObject& gameObject
        );

        void RemoveStaticInstance(const HRenderInstance& instance);

        void RemoveDynamicInstance(const HRenderInstance& instance);

        std::optional<HBatch> FindSimilarBatch(
            const types::u8* vertexBytes,
            types::usize vertexBytesCount,
            const types::u8* indexBytes,
            types::usize indexBytesCount
        );

        void Update();

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

        inline CStaticInstanceStorage& GetStaticInstanceStorage() const
        {
            return *_staticStorage;
        }

        inline CDynamicInstanceStorage& GetDynamicInstanceStorage() const
        {
            return *_dynamicStorage;
        }

    private:
        void MarkDirtyStatic();

        void MarkDirtyDynamic();

        types::boolean StaticBufferNeedsPacking();

        types::boolean DynamicBufferNeedsPacking();

        void RecalcBufferOffsetsAndResetCounters(types::u32* counterBuffer);

        template <
            typename TCPUObject,
            typename TCPUObjectHandle,
            typename TCPUObjectAllocator,
            typename TGPUObjectLayout
        >
        void PackInstancesToStagingBuffer(
            ERenderInstanceMotionType motionType,
            types::u32* counterBuffer,
            CUploader<TCPUObject, TCPUObjectHandle, TCPUObjectAllocator, TGPUObjectLayout>* uploader
        )
        {
            const SBufferView<SRenderInstanceData> bvInstances = uploader->GetData();
            for (usize instanceIdx = 0; instanceIdx < bvInstances.elementCount; instanceIdx++)
            {
                SRenderInstanceData& rid = bvInstances.elements[instanceIdx];
                SBatchData& bd = _batches->Get(rid.batch);

                if (bd.motionType != motionType)
                    continue;

                SGPURenderInstanceLayout gpuElementData;
                gpuElementData._use2D = 0.0f;
                gpuElementData._world = rid.worldMatrix;
                gpuElementData._skeletonIndex = -1;

                const usize batchIdx = _batches->GetHandleBufferIndex(rid.batch);
                const usize globElementIndex =
                    bd.bufferOffset +
                    counterBuffer[batchIdx];
                counterBuffer[batchIdx] += 1;

                uploader->WriteFieldToStaging(
                    globElementIndex,
                    gpuElementData
                );
            }
        }
    };
}