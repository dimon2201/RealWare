// batcher.hpp

#pragma once

#include <optional>
#include "object.hpp"
#include "render_instance_data.hpp"
#include "buffer_view.hpp"
#include "handle_allocator.hpp"
#include "batch_data.hpp"
#include "material_subsystem.hpp"
#include "skeleton_subsystem.hpp"
#include "batch_pool.hpp"
#include "static_render_instance_pool.hpp"
#include "dynamic_render_instance_pool.hpp"
#include "types.hpp"

#include "DELETE_THIS_FILE_ASAP.hpp"

namespace triton
{
    class cContext;
    template <typename TValue>
    class XLinearArray;
    class SGeometryView;
    struct SBatchData;
    class XBatchPool;
    class XStaticRenderInstancePool;
    class XDynamicRenderInstancePool;

    class XBatchSubsystem : public ISubsys
    {
        TRITON_OBJECT(XBatchSubsystem)

        XBatchPool* _batchPool = nullptr;
        XStaticRenderInstancePool* _staticInstancePool = nullptr;
        XDynamicRenderInstancePool* _dynamicInstancePool = nullptr;
        types::u32* _tempStaticCounterBuffer = nullptr;
        types::u32* _tempDynamicCounterBuffer = nullptr;
        types::boolean _bStaticBufferNeedsPacking = types::K_FALSE;
        types::boolean _bDynamicBufferNeedsPacking = types::K_FALSE;

    public:
        explicit XBatchSubsystem(cContext* context);
        ~XBatchSubsystem() override;

        std::optional<SBatchData::THandle> Create(
            EVertexBufferFormat vertexFormat,
            ERenderInstanceMotionType motionType,
            const SGeometryView& geometry
        );

        SBatchData& Get(const SBatchData::THandle& batch);

        void Destroy(const SBatchData::THandle& batch);

        std::optional<SStaticRenderInstanceData::THandle> AddStaticInstance(
            const SBatchData::THandle& batch,
            const SGameObjectData::THandle& gameObject
        );

        std::optional<SDynamicRenderInstanceData::THandle> AddDynamicInstance(
            const SBatchData::THandle& batch,
            const SGameObjectData::THandle& gameObject
        );

        void RemoveStaticInstance(const SStaticRenderInstanceData::THandle& instance);

        void RemoveDynamicInstance(const SDynamicRenderInstanceData::THandle& instance);

        void Init() override {}
        void Free() override {}
        void Update() override;

        inline const SBufferView<SBatchData>& GetBatches() const
        {
            return _batchPool->GetData();
        }

        inline XBatchPool* GetBatchPool() const
        {
            return _batchPool;
        }

        inline XStaticRenderInstancePool* GetStaticRenderInstancePool() const
        {
            return _staticInstancePool;
        }

        inline XDynamicRenderInstancePool* GetDynamicRenderInstancePool() const
        {
            return _dynamicInstancePool;
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