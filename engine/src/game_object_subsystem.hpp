// game_object_subsystem.hpp

#pragma once

#include <optional>
#include "object.hpp"
#include "DELETE_THIS_FILE_ASAP.hpp"
#include "game_object_data.hpp"
#include "graphics_buffer_formats.hpp"
#include "render_instance_data.hpp"
#include "model3d_data.hpp"
#include "batch_data.hpp"
#include "types.hpp"

namespace triton
{
    class cContext;
    template <typename TValue>
    class XLinearArray;
    class cVector3;
    struct SBatchInstance;
    class XGameObjectPool;

    class XGameObjectSubsystem : public ISubsys
    {
        TRITON_OBJECT(XGameObjectSubsystem)

        XGameObjectPool* _pool = nullptr;

    public:
        explicit XGameObjectSubsystem(cContext* context);
        ~XGameObjectSubsystem() override;

        std::optional<SGameObjectData::THandle> Create(const std::string& name);

        std::optional<SGameObjectData::THandle> Create(
            const std::string& name,
            const SModel3DData& model,
            const SBatchData::THandle& batchHandle
        );

        void Destroy(const SGameObjectData::THandle& gameObject);

        std::optional<SStaticRenderInstanceData::THandle> SetRenderableStatic(
            const SGameObjectData::THandle& gameObject,
            types::boolean bIsRenderable,
            const SBatchData::THandle& batchHandle = SBatchData::THandle()
        );

        std::optional<SDynamicRenderInstanceData::THandle> SetRenderableDynamic(
            const SGameObjectData::THandle& gameObject,
            types::boolean bIsRenderable,
            const SBatchData::THandle& batchHandle = SBatchData::THandle()
        );

        void SetMaterial(const SGameObjectData::THandle& gameObject, const SMaterialData::THandle& material);

        void SetWorldMatrix(const SGameObjectData::THandle& gameObject, const cMatrix4& matrix);

        void PlayAnimation(const SGameObjectData::THandle& gameObject, types::usize index);

        void Init() override;
        void Free() override;
        void Update() override;

        XGameObjectPool* GetPool() const
        {
            return _pool;
        }
    };
}