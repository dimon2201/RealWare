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

        void Init() override;
        void Free() override;
        void Update() override;

        std::optional<SStaticRenderInstanceData::THandle> SetRenderableStatic(
            const SGameObjectData::THandle& gameObject,
            const SBatchData::THandle& batch,
            const std::optional<SMaterialData::THandle>& existingMaterial = std::nullopt
        );
        
        std::optional<SDynamicRenderInstanceData::THandle> SetRenderableDynamic(
            const SGameObjectData::THandle& gameObject,
            const SBatchData::THandle& batch,
            const std::optional<SMaterialData::THandle>& existingMaterial = std::nullopt
        );

        inline XGameObjectPool* GetPool() const
        {
            return _pool;
        }
    };
}